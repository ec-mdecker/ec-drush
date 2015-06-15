<?php

require(dirname(__FILE__) . '/../terminus/terminus.user.api.inc');


function eaglecrk_drush_command() {
  $items = array();
  $items['eaglecrk-config'] = array(
    'description' => "Configure your server settings",
    'aliases' => array('ec-config'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  $items['eaglecrk-clone-site'] = array(
    'description' => "Clones a site into local development",
    'aliases' => array('ec-clone'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-sync-database'] = array(
    'description' => 'Download and overwrite an existing database',
    'aliases' => array('ec-sync'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-clear-terminus'] = array(
    'description' => "Remove stored variables such as configurations and login info.",
    'aliases' => array('ec-clear'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  $items['eaglecrk-reset-site'] = array(
    'description' => "Clears database and rebuilds the site.",
    'aliases' => array('ec-reset'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  return $items;
}

function drush_eaglecrk_config() {
  $settings = array();
  $options = array('Lamp', 'WAMP/XAMP', 'Other');
  $settings['env'] = drush_choice($options, 'What is your local environment?');
  $settings['dir'] = str_replace('\\', '/', drush_prompt(dt('Enter the directory desired for site files.')));
  $settings['db_name'] = drush_prompt(dt('Database Username'), 'root');
  $settings['db_pass'] = drush_prompt(dt('Database Password'), '', FALSE);

  drush_cache_set('eaglecrk-settings', $settings, 'pantheon-persist');
}

function drush_eaglecrk_clone_site($site_name = FALSE, $env = 'live') {
  $settings = drush_cache_get('eaglecrk-settings', 'pantheon-persist')->data;
  $site_uuid = terminus_get_site_uuid_by_name($site_name);
  $folder = trim(drush_prompt(dt('Site Folder')));
  $folder_array = explode('/', $folder);
  $url = implode('.', array_reverse($folder_array)) . '.local';
  $db = strtolower(preg_replace('/[^a-zA-Z0-9]/', '_', implode('_', $folder_array)));

  $git_clone = 'git clone ssh://codeserver.dev.' . $site_uuid . '@codeserver.dev.' . $site_uuid . '.drush.in:2222/~/repository.git ' . $settings['dir'] . '/' . $folder;
  shell_exec($git_clone);

  $files_folder = $settings['dir'] . "/$folder/sites/default/files";
  if (!file_exists($files_folder)) {
    shell_exec("sudo mkdir $files_folder");
  }
  shell_exec("sudo chmod 777 -R $files_folder");

  $db_file = drush_ec_download_backup($site_uuid, $env, 'latest', 'database', $settings['dir']);
  drush_print(dt('Database Downloaded'));
  shell_exec("gzip -d " . $settings['dir'] . "/$db_file");
  $db_file = $settings['dir'] . '/' . str_replace('.gz', '', $db_file);
  drush_print(dt('Database Extracted'));

  if (shell_exec("mysql -u " . $settings['db_name'] . " -e 'show databases like \"$db\"'")) {
    shell_exec("mysql -u " . $settings['db_name'] . " -e 'drop database $db'");
  }

  shell_exec("mysql -u " . $settings['db_name'] . " -e 'create database $db'");
  drush_print(dt('Local Database Created'));
  sleep(5);
  shell_exec("mysql -u " . $settings['db_name'] . " $db < $db_file");
  unlink($db_file);
  drush_print(dt('Database Imported'));

  shell_exec("sudo echo '\n127.0.0.1\t$url' >> /etc/hosts");
  $database_settings = file_get_contents(dirname(__FILE__) . '/settings.php.inc');
  $database_settings = str_replace('[database]', $db, $database_settings);
  $database_settings = str_replace('[dbname]', $settings['db_name'], $database_settings);
  $database_settings = str_replace('[dbpass]', $settings['db_pass'], $database_settings);
  $database_settings = str_replace('[url]', 'http://' . $url, $database_settings);
  $settings_file = $settings['dir'] . "/$folder/sites/default/settings.php";

  if (!file_exists($settings_file)) {
    shell_exec("sudo touch '$settings_file'; sudo chmod 777 $settings_file");
  }
  shell_exec("sudo echo '\n$database_settings' >> $settings_file");
  shell_exec("sudo chmod 444 $settings_file");

  $conf = file_get_contents(dirname(__FILE__) . '/site.conf');
  $conf = str_replace('[dir]', $settings['dir'], $conf);
  $conf = str_replace('[url]', $url, $conf);
  $conf = str_replace('[folder]', $folder, $conf);
  $conf_file = '/etc/apache2/sites-available/' . $folder_array[0] . '.conf';
  if (!file_exists($conf_file)) {
    shell_exec("sudo touch '$conf_file'; sudo chmod 777 $conf_file");
  }
  shell_exec("sudo echo '\n$conf' >> '$conf_file'");
  shell_exec("sudo a2ensite " . $folder_array[0] . ".conf; sudo service apache2 reload");

  shell_exec("cd " . $settings['dir'] . '/' . $folder . " && drush dl stage_file_proxy --destination='sites/default/modules' -y");
  shell_exec("cd " . $settings['dir'] . '/' . $folder . " && drush vset preprocess_js 0 --yes && drush vset preprocess_css 0 --yes && drush vset file_private_path 'sites/default/files' && drush vset file_temporary_path '/tmp' && drush cc all && drush vset cron_safe_threshold 0 && drush uli");
}

function drush_eaglecrk_sync_database($site_name = FALSE, $env = 'live') {
  shell_exec('sudo rm -rf sites/default/files/*');
  $settings = drush_cache_get('eaglecrk-settings', 'pantheon-persist')->data;
  $site_uuid = terminus_get_site_uuid_by_name($site_name);

  $stage_file_proxy = json_decode(shell_exec('drush vget stage_file_proxy_origin --format=json'), TRUE);
  $stage_file_proxy = $stage_file_proxy['stage_file_proxy_origin'];

  $status = json_decode(shell_exec("drush status --format='json'"), TRUE);
  $db = $status['db-name'];

  $db_file = drush_ec_download_backup($site_uuid, $env, 'latest', 'database', $settings['dir']);
  drush_print(dt('Database Downloaded'));
  shell_exec("gzip -d " . $settings['dir'] . "/$db_file");
  $db_file = $settings['dir'] . '/' . str_replace('.gz', '', $db_file);
  drush_print(dt('Database Extracted'));


  shell_exec("mysql -u " . $settings['db_name'] . " -e 'drop database $db'");
  shell_exec("mysql -u " . $settings['db_name'] . " -e 'create database $db'");
  sleep(5);
  shell_exec("mysql -u " . $settings['db_name'] . " $db < $db_file");
  unlink($db_file);
  shell_exec("drush vset preprocess_js 0 --yes; drush vset preprocess_css 0 --yes");
  shell_exec("drush vset file_private_path 'sites/default/files'; drush vset file_temporary_path '/tmp'");
  shell_exec("drush cc all; drush vset cron_safe_threshold 0; drush en stage_file_proxy -y");
  shell_exec("drush vset stage_file_proxy_origin '$stage_file_proxy'; drush uli");

  $css_injector = json_decode(shell_exec("drush pmi css_injector --format='json'"), TRUE);
  if ($css_injector['css_injector']['status']) {
    $injectors = explode("\n", shell_exec('mysql -u root -e "use ' . $db . '; select crid from css_injector_rule";'));
    unset($injectors[0]);
    mkdir('sites/default/files/css_injector');
    shell_exec("sudo chown www-data -R sites/default/files");
    foreach ($injectors as $crid) {
      if ((int) $crid > 0) {
        file_put_contents('sites/default/files/css_injector/css_injector_' . $crid . '.css', fopen($stage_file_proxy . '/sites/default/files/css_injector/css_injector_' . $crid . '.css', 'r'));
      }
    }
  }

  $js_injector = json_decode(shell_exec("drush pmi js_injector --format='json'"), TRUE);
  if ($js_injector['js_injector']['status']) {
    $injectors = explode("\n", shell_exec('mysql -u root -e "use ' . $db . '; select crid from js_injector_rule";'));
    unset($injectors[0]);
    mkdir('sites/default/files/js_injector');
    shell_exec("sudo chown www-data -R sites/default/files");
    foreach ($injectors as $crid) {
      if ((int) $crid > 0) {
        file_put_contents('sites/default/files/js_injector/js_injector_' . $crid . '.js', fopen($stage_file_proxy . '/sites/default/files/js_injector/js_injector_' . $crid . '.js', 'r'));
      }
    }
  }
}

function drush_eaglecrk_clear_terminus() {
  shell_exec("rm \$HOME/.drush/cache/pantheon-persist/terminus-last-email.cache");
}

function drush_eaglecrk_reset_site() {
  $settings = drush_cache_get('eaglecrk-settings', 'pantheon-persist')->data;
  $status = json_decode(shell_exec("drush status --format='json'"), TRUE);
  $db = $status['db-name'];

  shell_exec('mysql -u ' . $settings['db_name'] . ' -e "drop database ' . $db . '; create database ' . $db . ';"');
  shell_exec('xdg-open ' . $status['uri'] . '/install.php');
}

/**
 * Download a backup from a site.
 */
function drush_ec_download_backup($site_uuid = FALSE, $environment = FALSE, $bucket = FALSE, $element = FALSE, $mysql = FALSE) {
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