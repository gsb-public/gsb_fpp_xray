<?php

/**
 * Returns themed HTML for the FPP Xray form.
 */                              
function gsb_fpp_xray_preprocess_gsb_fpp_xray_form_data(&$variables) {

	$fpid = $variables['stuff'];
	if (empty($fpid)) {
    return;
  }

	_gsb_fpp_xray_get_fppinfo($fpid);

}

function _gsb_fpp_xray_get_fppinfo($fpid) {

  // fpid (621) -> uuid (df2fc47e-1346-4c40-84ce-dc0b27a43d39)

  dpm('fpp xray');

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'fieldable_panels_pane')
        ->propertyCondition('fpid', $fpid);
  $result = $query->execute();  
  $fpp = reset($result['fieldable_panels_pane']);
  $fpp_entity = fieldable_panels_panes_load($fpp->fpid);
  dpm($fpp_entity);  

  // uuid (df2fc47e-1346-4c40-84ce-dc0b27a43d39) -> list of did (1056, ...)

  dpm('panels_pane');

  $subtype = 'uuid:' . $fpp_entity->uuid; 
  $select = db_select('panels_pane','pp');
  $select->fields('pp');
  $select->condition('pp.subtype', $subtype);
  $results = $select->execute()->fetchAll();
  dpm($results);  

  // did (1056) -> entity_id, revision_id

  $dids = array();
  foreach($results as $result) {
    $dids[] = $result->did;
  }
  dpm($dids);

  dpm('panelizer_entity');

  $select = db_select('panelizer_entity','pe');
  $select->addField('pe', 'entity_id');
  $select->addField('pe', 'did');
  $select->condition('pe.did', $dids, 'IN');
  $select->distinct();
  $results = $select->execute()->fetchAll();
  dpm($results);

  // entity_id (300451) -> node

  $page_node_ids = array();
  foreach($results as $result) {
    $page_node_ids[] = $result->entity_id;
  }

  dpm('nodes');

  $nodes = node_load_multiple($page_node_ids);
  dpm($nodes); 

  _gsb_fpp_xray_download_nodes($nodes);  

}

function _gsb_fpp_xray_download_nodes($nodes) {

  // Add the headers needed to let the browser know this is a csv file download.
  //drupal_add_http_header('Content-Type', 'text/csv; utf-8');
  //drupal_add_http_header('Content-Disposition', 'attachment; filename = nodes.csv');

  // Open the file for writing.
  $fh = fopen('nodes.csv', 'w');

  // Add a header row
  fputcsv($fh, array(t('NID'), t('Title')));

  // Loop through our nodes and write the csv data.
  foreach($nodes as $node) {
    fputcsv($fh, array($node->nid, $node->title));
  }
 
  // Close & save the file.
  fclose($fh);

}

