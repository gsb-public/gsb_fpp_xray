<?php

/**
 * Returns themed HTML for the FPP Xray form.
 */
function gsb_fpp_xray_preprocess_gsb_fpp_xray_form_data(&$variables) {

  $action = '';

  $fpp_info = array();

  if (isset($variables['fpid'])) {
    $fpid = $variables['fpid'];
    $variables['action'] = 'byid';
    $variables['data'] = $fpid;

    $fpp_info = _gsb_fpp_xray_get_fppinfo_byids(array($fpid));

  } elseif (!empty($variables['bundle'])) {
    $bundle = $variables['bundle'];
    $variables['action'] = 'bybundle';
    $variables['data'] = $bundle;
    dpm('asdf');
    $fpids = _gsb_fpp_xray_get_fpid_by_bundle($bundle);
    dpm($fpids);
    $fpp_info = _gsb_fpp_xray_get_fppinfo_byids($fpids);
  }

  if (count($fpp_info) > 0) {
    _gsb_fpp_xray_dpm('fpp_info');
    _gsb_fpp_xray_dpm($fpp_info);
    $file_name = drupal_tempnam('temporary://', 'fpp_xray_csv_');
    $file_name = str_replace('temporary://', '', $file_name);
    $variables['file_name'] = $file_name;
  }
}

function _gsb_fpp_xray_get_sections_byid($ids) {

  $sections = array();

  foreach($ids as $id) {
    $term = taxonomy_term_load($id);
    $sections[] = $term->name;
  }
  return implode(',', array_values($sections));
}


function _gsb_fpp_xray_download_info($info, $file_name) {
  $fh = @fopen($file_name, 'w'); // The @ suppresses errors.
  // Add a header row
  @fputcsv($fh, [
    t('FPP Id'),
    t('FPP Title'),
    t('Reusable'),
    t('Node Id'),
    t('Node Title'),
    t('Sections'),
  ]);
  // Loop through our info and write the csv data.
  foreach ($info as $item) {
    $fpp = $item['fpp'];
    $nodes = $item['nodes'];
    if (count($nodes) == 0) {
      @fputcsv($fh, [$fpp->fpid, $fpp->admin_title, $fpp->reusable, '', '']);
    }
    else {
      foreach ($nodes as $node) {
        $sections = _gsb_fpp_xray_get_sections_byid(array_keys($node->workbench_access));
        @fputcsv($fh, [
          $fpp->fpid,
          $fpp->admin_title,
          $fpp->reusable,
          $node->nid,
          $node->title,
          $sections,
        ]);
      }
    }

  }
  // Close & save the file.
  @fclose($fh);

  $file_name = str_replace('temporary://', '', $file_name);
  return $file_name;

}
