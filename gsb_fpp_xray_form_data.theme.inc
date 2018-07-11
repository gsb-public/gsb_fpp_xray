<?php

/**
 * Returns themed HTML for the FPP Xray form.
 */
function gsb_fpp_xray_preprocess_gsb_fpp_xray_form_data(&$variables) {

  $action = '';

  $info = array();

  $fpid = $variables['fpid'];
  if (!empty($fpid)) {
    list($returned_fpp, $returned_nodes) = _gsb_fpp_xray_get_fppinfo_byid($fpid);
    $info[$fpid]['fpp'] = $returned_fpp;
    $info[$fpid]['nodes'] = $returned_nodes;
    $variables['action'] = 'byid';
    $variables['data'] = $fpid;
  }

  $bundle = $variables['bundle'];
  if (!empty($bundle)) {
    $fpps = _gsb_fpp_xray_get_fppinfo_bybundle($bundle);
    foreach ($fpps as $fpid) {
      list($returned_fpp, $returned_nodes) = _gsb_fpp_xray_get_fppinfo_byid($fpid);
      $info[$fpid]['fpp'] = $returned_fpp;
      $info[$fpid]['nodes'] = $returned_nodes;
    }
    $variables['action'] = 'bybundle';
    $variables['data'] = $bundle;
  }

  if (count($info) > 0) {
    _gsb_fpp_xray_dpm('info');
    _gsb_fpp_xray_dpm($info);
    $file_name = drupal_tempnam('temporary://', 'fpp_xray_csv_');
    $file_name = str_replace('temporary://', '', $file_name);
    $variables['file_name'] = $file_name;
  }

}

function _gsb_fpp_xray_get_fppinfo_byentity($fpp_entity) {

  $dids = array();
  $pids = array();

  $subtypes = array('current:'. $fpp_entity->fpid, 'uuid:' . $fpp_entity->uuid, 'vuuid:' . $fpp_entity->vuuid);
  $select = db_select('panels_pane','pp');
  $select->fields('pp');
  $select->condition('pp.subtype', $subtypes, 'IN');
  $results = $select->execute()->fetchAll();

  foreach($results as $result) {
    $dids[] = $result->did;
    $pids[] = $result->pid;
  }
  //dpm($dids);

  if (count($dids) == 0) {
    return array( $fpp_entity, array() );
  }

  //dpm('panelizer_entity');

  // Look up nodes by dids

  $select = db_select('panelizer_entity','pe');
  $select->addField('pe', 'entity_id');
  $select->addField('pe', 'did');
  $select->condition('pe.did', $dids, 'IN');
  $select->distinct();
  $results = $select->execute()->fetchAll();
  //dpm($results);

  // entity_id (300451) -> node

  $page_node_ids = array();
  foreach($results as $result) {
    $page_node_ids[$result->did] = $result->entity_id;
  }

  $valid_nodes = findValidNodes_new($page_node_ids, $pids, $fpp_entity);

  return array($fpp_entity, $valid_nodes);
}

function findValidNodes_new($page_node_ids, $pids, $fpp_entity) {

  // select nid from node table
  $select = db_select('node','node');
  $select->addField('node', 'nid');
  $select->addField('node', 'vid');
  $select->condition('node.nid', $page_node_ids, 'IN');
  $select->distinct();
  $results = $select->execute()->fetchAll();

  $vids = array();
  foreach($results as $result) {
    $vids[] = $result->vid;
  }

  /*
  $result = db_query("SELECT * FROM {panelizer_entity} WHERE entity_type = :entity_type AND revision_id IN (:vids)", array(':entity_type' => 'node', ':vids' => $vids));
  while ($panelizer = $result->fetchObject()) {
    $dids[] = $panelizer->did;
  }
  */
  /*
  if ($this->supports_revisions) {
    $result = db_query("SELECT * FROM {panelizer_entity} WHERE entity_type = :entity_type AND revision_id IN (:vids)", array(':entity_type' => $this->entity_type, ':vids' => $vids));
  }
  else {
    $result = db_query("SELECT * FROM {panelizer_entity} WHERE entity_type = :entity_type AND entity_id IN (:ids)", array(':entity_type' => $this->entity_type, ':ids' => $ids));
  }
  */

  $select = db_select('panelizer_entity','pe');
  $select->addField('pe', 'did');
  $select->addField('pe', 'revision_id');
  $select->addField('pe', 'entity_type');
  $select->condition('pe.entity_type', 'node');
  $select->condition('pe.revision_id', $vids, 'IN');
  $select->distinct();
  $results = $select->execute()->fetchAll();
  foreach($results as $result) {
    $dids[] = $result->did;
  }

  $displays = panels_load_displays($dids);

  $valid_nodes = array();
  foreach ($displays as $display) {
    foreach ($display->content as $content) {
      foreach($pids as $pid) {
        if ($content->pid == $pid) {
          if (!empty($page_node_ids[$content->did])) {
            $valid_nodes[] = $page_node_ids[$content->did];
          }
          break;
        }
      }
    }
  }

  return $valid_nodes;
}

function findValidNodes($page_node_ids, $pids) {

  $nodes = node_load_multiple($page_node_ids);

  // Check the panel panes of the nodes for matching pids

  $valid_nodes = array();
  foreach($nodes as $node) {
    foreach($pids as $pid) {
      if (!empty($node->panelizer['page_manager']->display->content[$pid])) {
        $valid_nodes[$node->nid] = $node;
        break;
      }
    }
  }

  return $valid_nodes;
}

function _gsb_fpp_xray_get_fppinfo_byid($fpid) {

  // fpid (621) -> uuid (df2fc47e-1346-4c40-84ce-dc0b27a43d39)

  //dpm('fpp xray by id, id = ' . $fpid);

  $select = db_select('fieldable_panels_panes_revision','fpprev');
  $select->addField('fpprev', 'fpid');
  $select->addField('fpprev', 'vid');
  $select->condition('fpprev.fpid', $fpid);
  $select->distinct();
  $results = $select->execute()->fetchAll();

  $returned_fpp = array();
  $returned_nodes = array();

  foreach($results as $result) {

    $vid = $result->vid;

    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'fieldable_panels_pane')
      ->propertyCondition('fpid', $fpid);
    $result = $query->execute();
    $fpp = reset($result['fieldable_panels_pane']);
    $fpp_entity = fieldable_panels_panes_load($fpp->fpid, $vid);

    list($found_fpp, $found_nodes) = _gsb_fpp_xray_get_fppinfo_byentity($fpp_entity);
    $returned_fpp = $found_fpp;

    array_walk($found_nodes, function($value, $key) use (&$returned_nodes) {
      $returned_nodes[$key] = $value;
    });

  }

  return array($returned_fpp, $returned_nodes);
}

function _gsb_fpp_xray_get_fppinfo_bybundle($bundle) {

  _gsb_fpp_xray_dpm('fpp xray by bundle, bundle = ' . $bundle);

  $entity_type = 'fieldable_panels_pane';

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', $entity_type)
    ->propertyCondition('bundle', $bundle);
  $result = $query->execute();

  return array_keys($result[$entity_type]);

}

function _gsb_fpp_xray_download_info($info, $file_name) {

  $fh = @fopen($file_name, 'w'); // The @ suppresses errors.

  // Add a header row
  @fputcsv($fh, array(t('FPP Id'), t('FPP Title'), t('Reusable'), t('Node Id'), t('Node Title'), t('Sections'),));

  // Loop through our info and write the csv data.
  foreach($info as $item) {
    $fpp = $item['fpp'];
    $nodes = $item['nodes'];

    if (count($nodes) == 0) {
      @fputcsv($fh, array($fpp->fpid, $fpp->admin_title, $fpp->reusable, '', ''));
    } else {
      foreach($nodes as $node) {
        $sections = _gsb_fpp_xray_get_sections_byid(array_keys($node->workbench_access));
        @fputcsv($fh, array($fpp->fpid, $fpp->admin_title, $fpp->reusable, $node->nid, $node->title, $sections));
      }
    }

  }

  // Close & save the file.
  @fclose($fh);

  $file_name = str_replace('temporary://', '', $file_name);
  return $file_name;

}

function _gsb_fpp_xray_get_sections_byid($ids) {

  $sections = array();

  foreach($ids as $id) {
    $term = taxonomy_term_load($id);
    $sections[] = $term->name;
  }

  return implode(',', array_values($sections));
}

