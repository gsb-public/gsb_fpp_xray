<?php


/**
 * @param $subtypes
 *
 * @return mixed
 * [
 *  {
 *    "pid":"13771",
 *    "did":"2256",
 *    "panel":"mainbottom",
 *    "type":"fieldable_panels_pane",
 *    "subtype":"uuid:98d70898-736b-4d45-ac04-c2fa02d37de3",
 *    "shown":"1",
 *    "access":"a:0:{}",
 *    "configuration":"a:1:{s:9:\"view_mode\";s:4:\"full\";}",
 *    "cache":"a:0:{}",
 *    "style":"a:0:{}",
 *    "css":"a:0:{}",
 *    "extras":"a:0:{}",
 *    "position":"0",
 *    "locks":"a:0:{}",
 *    "uuid":null
 *    }
 * ]
 */
function query_fpp_panels_panes($subtypes) {
  $query = db_select('panels_pane', 'pp');
  $query->fields('pp');
  $query->condition('pp.subtype', $subtypes, 'IN');
  $output = $query->execute()->fetchAll();
  return $output;
}

/**
 * @param $dids
 *
 * @return mixed
 *  [
 *    {
 *      "entity_id":"301551",
 *      "did":"1536"
 *    },
 *    {
 *      "entity_id":"301551",
 *      "did":"24201"
 *    },
 *  ]
 */
function query_fpp_panelizer_entity($dids) {
  $query = db_select('panelizer_entity', 'pe');
  $query->addField('pe', 'entity_id');
  $query->addField('pe', 'did');
  $query->condition('pe.did', $dids, 'IN');
  $query->distinct();
  $output = $query->execute()->fetchAll();
  return $output;
}

/**
 * @param $fpid
 *
 *
 * @return fpp revision fpid, version id
 * [
 *  {"fpid":"1116","vid":"1171"},
 *  {"fpid":"1116","vid":"17546"}
 * ]
 */
function query_fpp_revisions($fpids) {
  $query = db_select('fieldable_panels_panes_revision', 'fpp_rev');
  $query->addField('fpp_rev', 'vid');
  $query->addField('fpp_rev', 'fpid');
  $query->condition('fpp_rev.fpid', $fpids, 'IN');
  $query->distinct();
  $output = $query->execute()->fetchAll();
  return $output;
}

/**
 * @param $fpids
 *
 * @return array
 * [$output['fpp_entity'], $output['returned_nodes']]
 */
function _gsb_fpp_xray_get_fppinfo_byids($fpids) {
  $fpp_revisions = query_fpp_revisions($fpids);
  foreach($fpp_revisions as $fpp_revision) {
    $fpp_info[$fpp_revision->fpid][] = $fpp_revision->vid;
  }
  foreach($fpids as $fpid) {
    $output = [];
    foreach ($fpp_info[$fpid] as $fpp_revision_vid) {
      //$result = query_fpp_entity_fpp($fpid);
      //$fpp = $result['fieldable_panels_pane'][$fpid];
      $output['fpp_entity'] = fieldable_panels_panes_load($fpid, $fpp_revision_vid);

      // Look for 'current:' subtype
      $subtypes = [
        'current:' . $output['fpp_entity']->fpid,
        'uuid:' . $output['fpp_entity']->uuid,
        'vuuid:' . $output['fpp_entity']->vuuid
      ];

      $panels_panes = query_fpp_panels_panes($subtypes);
      //dpm(json_encode($results));
      $panels_pane_dids = [];
      $panels_pane_pids = [];

      foreach ($panels_panes as $panels_pane) {
        $panels_pane_dids[] = $panels_pane->did;
        $panels_pane_pids[] = $panels_pane->pid;
      }

      $output['returned_nodes'] = [];
      if (count($panels_pane_dids) != 0) {
        $panelizer_entities = query_fpp_panelizer_entity($panels_pane_dids);
        $page_node_ids = [];

        foreach ($panelizer_entities as $panelizer_entity) {
          $page_node_ids[] = $panelizer_entity->entity_id;
        }
        $nodes = node_load_multiple($page_node_ids);

        // Check the panel panes of the nodes for matching pids

        foreach ($nodes as $node) {
          foreach ($panels_pane_pids as $panels_pane_pid) {
            if (!empty($node->panelizer['page_manager']->display->content[$panels_pane_pid])) {
              $output['returned_nodes'][$node->nid] = $node;
              break;
            }
          }
        }
      }
    }

    $output_data[$fpid]['fpp'] = $output['fpp_entity'];
    $output_data[$fpid]['nodes'] = $output['returned_nodes'];
    //return [$output['fpp_entity'], $output['returned_nodes']];
  }
  return $output_data;
}