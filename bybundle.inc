<?php
/**
 * @param $bundle
 *
 * @return array
 */

function _gsb_fpp_xray_get_fpid_by_bundle($bundle) {
  // _gsb_fpp_xray_dpm('fpp xray by bundle, bundle = ' . $bundle);
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'fieldable_panels_pane')
    ->propertyCondition('bundle', $bundle);
  $result = $query->execute();
  $output = array_keys($result['fieldable_panels_pane']);
  dpm('bundle_data');
  _gsb_fpp_xray_dpm($output);
  return $output;
}
