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

  // uuid (df2fc47e-1346-4c40-84ce-dc0b27a43d39) -> list of did (1056, ...)

  //dpm('panels_pane');

  $fpid = $fpp_entity->fpid;

  $subtype = 'uuid:' . $fpp_entity->uuid;
  $select = db_select('panels_pane','pp');
  $select->fields('pp');
  $select->condition('pp.subtype', $subtype);
  $results = $select->execute()->fetchAll();
  //dpm($results);

  // did (1056) -> entity_id, revision_id

  $dids = array();
  $pids = array();

  foreach($results as $result) {
    $dids[] = $result->did;
    $pids[] = $result->pid;
  }
  //dpm($dids);

  // Look for 'current:' subtype

  $subtype = 'current:' . $fpid;
  $select = db_select('panels_pane','pp');
  $select->fields('pp');
  $select->condition('pp.subtype', $subtype);
  $results = $select->execute()->fetchAll();
  //dpm($results);

  foreach($results as $result) {
    $dids[] = $result->did;
    $pids[] = $result->pid;
  }
  //dpm($dids);

  // Look for 'uuid:' subtype

  $subtype = 'uuid:' . $fpp_entity->uuid;
  $select = db_select('panels_pane','pp');
  $select->fields('pp');
  $select->condition('pp.subtype', $subtype);
  $results = $select->execute()->fetchAll();
  //dpm($results);

  foreach($results as $result) {
    $dids[] = $result->did;
    $pids[] = $result->pid;
  }
  //dpm($dids);

  // Look for 'vuuid:' subtype

  $subtype = 'vuuid:' . $fpp_entity->vuuid;
  $select = db_select('panels_pane','pp');
  $select->fields('pp');
  $select->condition('pp.subtype', $subtype);
  $results = $select->execute()->fetchAll();
  //dpm($results);

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
    $page_node_ids[] = $result->entity_id;
  }

  //dpm('nodes');

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

  return array($fpp_entity, $valid_nodes);
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


