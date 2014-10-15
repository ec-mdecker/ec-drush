<?php

function eaglecrk_drush_command() {
  $items = array();
  $items['eaglecrk-config'] = array(
    'description' => "Configure your server settings",
    'aliases' => array('ec-config'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  $items['eaglecrk-clone-site'] = array(
    'description' => "Clones a site into local development",
    'aliases' => array('ec-csite'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-show-config'] = array(
    'description' => "Displays current configurations.",
    'aliases' => array('ec-show-config'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-restore-database'] = array(
    'description' => 'Download and overwrite an existing database',
    'aliases' => array('ec-redb'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['ec-test'] = array(
    'description' => "Clones a site into local development",
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  return $items;
}

function drush_eaglecrk_config() {
  drush_print('Enter the directory for you mysql. This folder must contain a mysql.exe file.');
  $mysql = str_replace('\\', '/', drush_prompt(dt(''), NULL, TRUE));
  while (!file_exists($mysql)) {
    $mysql = drush_prompt('', NULL, TRUE);
  }
  drush_print(dt('Enter the directory desired for site files.'));
  $git = str_replace('\\', '/', drush_prompt(dt(''), NULL, TRUE));
  while (!file_exists($git)) {
    $git = drush_prompt('', NULL, TRUE);
  }
  $options = array('WAMP/XAMP', 'Acquia', 'Other');
  $local_env = drush_choice($options, 'What is your local environment?');
  if ($local_env === 0) {
    $db_user = drush_prompt(dt('Username for the database.'), NULL, FALSE);
    if (is_null($db_user)) {
      $db_user = 'root';
    }

    $db_pass = drush_prompt(dt('Password for the database.'), NULL, FALSE);
    if (is_null($db_user)) {
      $db_pass = '';
    }
    drush_cache_set('eaglecrk-db-user', $db_user, 'pantheon-persist');
    drush_cache_set('eaglecrk-db-pass', $db_pass, 'pantheon-persist');

  }
  drush_cache_set('eaglecrk-local-env', $local_env, 'pantheon-persist');
  drush_cache_set('eaglecrk-mysql', $mysql, 'pantheon-persist');
  drush_cache_set('eaglecrk-git', $git, 'pantheon-persist');

}

function drush_eaglecrk_show_config() {
  $mysql = drush_cache_get('eaglecrk-mysql', 'pantheon-persist')->data;
  $git = drush_cache_get('eaglecrk-git', 'pantheon-persist')->data;
  $db_user = drush_cache_get('eaglecrk-db-user', 'pantheon-persist')->data;
  $db_pass = drush_cache_get('eaglecrk-db-pass', 'pantheon-persist')->data;
  $loacal_env = drush_cache_get('eaglecrk-local-env', 'pantheon-persist')->data;
  drush_print('Mysql Dir: ' . $mysql);
  drush_print('File Dir: ' . $git);
  drush_print('Username: ' . $db_user);
  drush_print('Password: ' . $db_pass);
  $options = array('WAMP/XAMP', 'Acquia', 'Other');
  drush_print('Local Environment: ' . $options[$local_env]);
}

function drush_eaglecrk_clone_site($site_name = FALSE, $environment = 'dev') {
  if (is_null(drush_cache_get('eaglecrk-mysql', 'pantheon-persist')->data)) {
    drush_eaglecrk_config();
  }
  $mysql = drush_cache_get('eaglecrk-mysql', 'pantheon-persist')->data;
  $git = drush_cache_get('eaglecrk-git', 'pantheon-persist')->data;
  $db_user = drush_cache_get('eaglecrk-db-user', 'pantheon-persist')->data;
  $db_pass = drush_cache_get('eaglecrk-db-pass', 'pantheon-persist')->data;

  $site_uuid = terminus_get_site_uuid_by_name($site_name);
  $bucket = 'latest';
  $element = 'database';
  $file_name = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $mysql);

  if ($file_name) {
    $buffer_size = 4096;
    $out_file_name = str_replace('.gz', '', $file_name);
    $file = gzopen($mysql . '/' . $file_name, 'rb');
    $out_file = fopen($mysql . '/' . $out_file_name, 'wb');
    while (!gzeof($file)) {
      fwrite($out_file, gzread($file, $buffer_size));
    }
    fclose($out_file);
    gzclose($file);
    unlink($mysql . '/' . $file_name);

    if (!is_null($db_user)) {
      $db_name = drush_prompt(dt('What is the name of the new database'), NULL, TRUE);
      shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -e "create database ' . $db_name . '";');
      shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -p ' . $db_name . ' < ' . $mysql . '/' . $out_file_name);
      unlink($mysql . '/' . $out_file_name);
    }
    $git_clone = 'git clone ssh://codeserver.dev.' . $site_uuid . '@codeserver.dev.' . $site_uuid . '.drush.in:2222/~/repository.git ' . $git . '/' . $site_name . '-' . $environment;
    shell_exec($git_clone);

    if (!is_null($db_user)) {
      $settings_file = $git . '/' . $site_name . '-' . $environment . '/sites/default/settings.php';
      if (!file_exists($settings_file)) {
        copy($git . '/' . $site_name . '-' . $environment . '/sites/default/default.settings.php', $settings_file);
      }

      $add_string = "\$databases = array(\n\t'default' => array(\n\t\t'default' => array(";
      $add_string .= "\n\t\t\t'driver' => 'mysql',\n\t\t\t'database' => '" . $db_name . "',";
      $add_string .= "\n\t\t\t'username' => '" . $db_user . "',\n\t\t\t'password' => '" . $db_pass . "',";
      $add_string .= "\n\t\t\t'host' => 'localhost',\n\t\t\t'prefix' => '',";
      $add_string .= "\n\t\t\t'collation' => 'utf8_general_ci',\n\t\t)\n\t) \n);";


      file_put_contents($settings_file, $add_string, FILE_APPEND);
      if (file_exists('C:\Windows\System32\drivers\etc\hosts')) {
        file_put_contents('C:\Windows\System32\drivers\etc\hosts', "\n127.0.0.1 " . $site_name . '-' . $environment, FILE_APPEND);
      }

      if (file_exists('C:\wamp\bin\apache\apache2.4.9\conf\extra\httpd-vhosts.conf')) {
        $vhost = "\n\n<VirtualHost *:80>\n\tDocumentRoot \"" . $git . '/' . $site_name . '-' . $environment . "\"\n\tServerName " . $site_name . '-' . $environment . "\n</VirtualHost>";
        file_put_contents('C:\wamp\bin\apache\apache2.4.9\conf\extra\httpd-vhosts.conf', $vhost, FILE_APPEND);
      }
    }

    $files = drush_choice(array('Yes', 'No'), 'Download the files?');
    if ($files === 0) {
      $element = 'files';
      $dir = $git . '/' . $site_name . '-' . $environment . '/sites/default';
      $file_name = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $dir);;

      $p = new PharData($dir . '/' . $file_name);
      unlink($dir . '/' . $file_name);
      $p->decompress();

      $phar = new PharData($dir . '/' . str_replace('.gz', '', $file_name));
      $phar->extractTo($dir);

      rename($dir . '/files_' . $environment, $dir . '/files');
    }
  }
  else {
    drush_terminus_pantheon_site_make_backup($site_uuid, $environment);
    drush_print('A backup has been started. Please wait a moment and try this command again.');
  }

}

function drush_eaglecrk_ec_test() {

}


/**
 * Download a backup from a site.
 */
function drush_eaglecrk_pantheon_site_download_backup($site_uuid = FALSE, $environment = FALSE, $bucket = FALSE, $element = FALSE, $mysql = FALSE) {
  $session_data = terminus_bootstrap();
  if ($session_data === FALSE) {
    return FALSE;
  }
  extract($session_data);

  if (!$site_uuid = terminus_site_input($site_uuid)) {
    drush_log('You must supply a site UUID', 'failed');
    return FALSE;
  }

  if (!$environment) {
    $environment = terminus_session_select_data('environment', $site_uuid);
    if (!$environment) {
      drush_log('You must supply an environment', 'failed');
      return FALSE;
    }
  }
  if (!$element) {
    $element = terminus_session_select_data('element');
    if (!$element) {
      drush_log('You must supply an element', 'failed');
      return FALSE;
    }
  }

  // Retrieve the latest bucket
  if ($bucket == 'latest') {
    $bucket = terminus_latest_bucket($site_uuid, $environment, $element);
  }
  if (!$bucket) {
    $bucket = terminus_session_select_data('bucket', $site_uuid, $environment, $element);
    if (!$bucket) {
      drush_log('You must supply a bucket', 'failed');
      return FALSE;
    }
  }
  if (!$mysql) {
    $mysql = terminus_session_select_data('destination');
    if (!$mysql) {
      drush_log('You must supply a destination', 'failed');
      return FALSE;
    }
  }

  $result = terminus_api_backup_download_url($site_uuid, $environment, $bucket, $element);
  $data = json_decode($result['json']);
  $filename = strstr(basename($data->url), '?', '_');

  drush_log("Downloading " . $filename . "...");

  // Copied from pm code
  $cache_duration = 86400 * 365;
  $path = _drush_download_file($data->url, $mysql . DIRECTORY_SEPARATOR . $filename, $cache_duration);
  if ($path || drush_get_context('DRUSH_SIMULATE')) {
    drush_log("Downloading " . $filename . " was successful.");
  }
  else {
    return drush_set_error('DRUSH_PSITE_DOWNLOAD_FAILED', 'Unable to download ' . $filename . ' to ' . $mysql . ' from ' . $data->url);
  }

  return $filename;
}


function drush_eaglecrk_restore_database($site_name = NULL, $environment = 'dev'){
  if (is_null(drush_cache_get('eaglecrk-mysql', 'pantheon-persist')->data)) {
    drush_eaglecrk_config();
  }
  $mysql = drush_cache_get('eaglecrk-mysql', 'pantheon-persist')->data;
  $db_user = drush_cache_get('eaglecrk-db-user', 'pantheon-persist')->data;

  $site_uuid = terminus_get_site_uuid_by_name($site_name);
  $bucket = 'latest';
  $element = 'database';
  $file_name = drush_eaglecrk_pantheon_site_download_backup($site_uuid, $environment, $bucket, $element, $mysql);
  if ($file_name) {
    $buffer_size = 4096;
    $out_file_name = str_replace('.gz', '', $file_name);
    $file = gzopen($mysql . '/' . $file_name, 'rb');
    $out_file = fopen($mysql . '/' . $out_file_name, 'wb');
    while (!gzeof($file)) {
      fwrite($out_file, gzread($file, $buffer_size));
    }
    fclose($out_file);
    gzclose($file);
    unlink($mysql . '/' . $file_name);


    if (!is_null($db_user)) {
      $db_name = drush_prompt(dt('What is the name of the database?'), NULL, TRUE);
      shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -e "drop database ' . $db_name . '";');
      shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -e "create database ' . $db_name . '";');
      shell_exec($mysql . '/mysql.exe -u ' . $db_user . ' -p ' . $db_name . ' < ' . $mysql . '/' . $out_file_name);
      unlink($mysql . '/' . $out_file_name);
    }
  }
}