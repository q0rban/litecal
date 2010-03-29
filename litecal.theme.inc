<?php

/**
 * Theme a timespan item.
 */
function theme_litecal_timeitem($item, $granularity) {
  $left = number_format($item->start / $granularity * 100, 2);
  $width = number_format($item->size / $granularity * 100, 2);

  $attr = array('style' => "left:{$left}%; width:{$width}%;");

  $attr['class'] = 'litecal-item';
  $attr['class'] .= $item->starts ? ' starts' : '';
  $attr['class'] .= $item->ends ? ' ends' : '';
  $attr['class'] .= ' crayon crayon-'. theme('seed_crayon', $item->id);
  $popup = theme('seed_crayon_popup', $item->id, $item->data);

  if (!empty($item->url)) {
    return l("$popup<span class='litecal-data'>{$item->data}</span>", $item->url, array('attributes' => $attr, 'html' => TRUE));
  }
  else {
    $attr = drupal_attributes($attr);
    return "<div {$attr}>$popup<span class='litecal-data'>{$item->data}</span></div>";
  }
}

/**
 * Theme all timeslots for a given timespan.
 */
function theme_litecal_timeslots($timespan, $quickadd = array()) {
  switch ($timespan->unit) {
    case 'days':
      $format = 'j';
      break;
  }

  $slots = array();
  $date = drupal_clone($timespan->from);

  for ($i = 0; $i < $timespan->granularity; $i++) { 
    $slots[] = theme('litecal_timeslot', $timespan, $i, $date, $format, $quickadd);
    date_modify($date, "+1 {$timespan->unit}");
  }
  return $slots;
}

/**
 * Theme a single timeslot.
 */
function theme_litecal_timeslot($timespan, $start, $date, $format, $quickadd = array()) {
  $add = '';
  $attr = array('style' => '');
  $link_attr = array('class' => 'label');

  // Position
  if ($start < $timespan->granularity - 1) {
    $attr['style'] .= ' left:'. number_format($start / $timespan->granularity * 100, 2) .'%;';
  }
  // We position last items differently since slots often use borders and need tight alignment.
  else {
    $attr['style'] .= ' right:0%;';
  }

  // Width
  $attr['style'] .= ' width:'. number_format(1 / $timespan->granularity * 100, 2) .'%';

  // Classes
  $attr['class'] = 'litecal-slot rows-'. count($timespan->built);

  // Add class for today's slot.
  static $today;
  $today = !isset($today) ? date_format_date(date_now(), 'custom', 'Y-m-d') : $today;
  if ($today == date_format_date($date, 'custom', 'Y-m-d')) {
    $attr['class'] .= ' litecal-slot-today';
  }

  // If this timeslot is outside of the timespan's real time range,
  // add a class so it can be displayed accordingly.
  if (!litecal_date_between($date, $timespan->real_from, $timespan->real_to)) {
    $attr['class'] .= ' litecal-slot-gutter';
  }

  $attr = drupal_attributes($attr);

  // Quickadd
  if (!empty($quickadd['type'])) {
    $type = str_replace('_', '-', $quickadd['type']);
    $item = menu_get_item("node/add/{$type}");
    if ($item && $item['access']) {
      $options = array('query' => "edit[{$quickadd['field']}][0][value]=". date_convert($date, DATE_OBJECT, DATE_ISO) ."&destination=" . $_GET['q']);
      $link_attr['href'] = url("node/add/{$type}", $options);
      $add = "<span class='add'>". t('+ Add') ."</span>";
    }
  }
  $link_attr = drupal_attributes($link_attr);

  $formatted = date_format_date($date, 'custom', $format);

  return "<div {$attr}>
    <a $link_attr>{$add}<span class='num'>{$formatted}</span></a>
  </div>";
}

/**
 * Theme a header, like days of the week for a month.
 */
function theme_litecal_header($label, $start, $granularity) {
  $left = number_format($start / $granularity * 100, 2);
  $width = number_format(1 / $granularity * 100, 2);

  $attr = array('style' => "left:{$left}%; width:{$width}%;");
  $attr = drupal_attributes($attr);

  return "<div class='litecal-label' {$attr}>{$label}</div>";
}

/**
 * Use mod math to generate an integer for crayon coloring.
 */
function theme_seed_crayon($id) {
  static $crayon;
  static $strings;

  // Initialize static vars.
  if (!isset($crayon)) {
    $crayon = array();
    $strings = array();
  }

  // Do a string to int conversion if necessary
  if (!is_numeric($id)) {
    if (!isset($strings[$id])) {
      // @TODO: Determine the best distinct string to distinct int conversion here.
      $strings[$id] = abs(crc32($id));
    }
    $id = $strings[$id];
  }

  // Return the modulo crayon color id
  if (isset($crayon[$id])) {
    return $crayon[$id];
  }
  else {
    $modulo = $id % 16;

    // Try 5 times to avoid color collision
    $try = 0;
    while (in_array($modulo, $crayon, TRUE) && $try < 5) {
      $modulo = ($modulo + 1) % 16;
      $try++;
    }

    $crayon[$id] = $modulo;
    return $crayon[$id];
  }
}

/**
 * A generalized hover popup style.
 */
function theme_seed_crayon_popup($id, $label, $color = NULL) {
  $style = $output = '';

  $crayon = !empty($id) ? theme('seed_crayon', $id) : NULL;
  if (!empty($color)) {
    $style = (strpos($color, '#') !== FALSE) ? "background-color: {$color}" : "background-color: #{$color}";
  }
  $content = empty($content) ? $label : $content;

  $output .= "<span class='crayon-popup'>";
  $output .= "<span class='crayon-popup-label'>";
  if (isset($crayon)) {
    $output .= "<span class='crayon-marker crayon-{$crayon}' style='{$style}'></span>";
  }
  $output .= $label;
  $output .= "</span>";
  $output .= "</span>";
  return $output;
}

/**
 * A small swatch of color with label that has a popup with more info on hover.
 */
function theme_seed_crayon_swatch($id, $label, $popup = NULL, $path = NULL, $color = NULL) {
  $popup = empty($popup) ? $label : $popup;
  $popup = theme('seed_crayon_popup', $id, $popup, $color);

  $crayon = theme('seed_crayon', $id);
  if (!empty($color)) {
    $style = (strpos($color, '#') !== FALSE) ? "background-color: {$color}" : "background-color: #{$color}";
  }
  $options = array(
    'html' => true,
    'attributes' => array(
      'class' => 'crayon crayon-swatch crayon-'. $crayon,
      'style' => !empty($style) ? $style : NULL,
    ),
  );

  $content = "<span class='label'>$label</span>$popup";

  if (!empty($path)) {
    return l($content, $path, $options);
  }
  else {
    $attr = drupal_attributes($options['attributes']);
    return "<span $attr>$content</span>";
  }
}