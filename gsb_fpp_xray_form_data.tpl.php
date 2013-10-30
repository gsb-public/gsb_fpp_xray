<?php //dpm($variables); ?>

<?php if (!empty($variables['fpid']) || !empty($variables['bundle'])) { ?>
	<div>Download your csv formatted output <a href="/admin/config/gsb/fpp-xray/download/<?php print $variables['file_name'] ?>.csv">here</a>.</div>
<?php } ?>

