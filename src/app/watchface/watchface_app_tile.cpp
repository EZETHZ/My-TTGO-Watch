/****************************************************************************
 *   Aug 3 12:17:11 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include <TTGO.h>

#include "watchface_app.h"
#include "watchface_app_tile.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"
#include "gui/widget.h"
#include "gui/widget_styles.h"

#include "hardware/powermgm.h"
#include "hardware/display.h"
#include "hardware/touch.h"

lv_task_t *watchface_tile_task;
volatile bool watchface_active = false;
volatile bool watchface_enable_after_wakeup = false;

uint32_t watchface_app_tile_num;
lv_obj_t *watchface_app_tile = NULL;
lv_obj_t *watchface_dial_img = NULL;
lv_obj_t *watchface_hour_img = NULL;
lv_obj_t *watchface_min_img = NULL;
lv_obj_t *watchface_sec_img = NULL;
lv_obj_t *watchface_hour_s_img = NULL;
lv_obj_t *watchface_min_s_img = NULL;
lv_obj_t *watchface_sec_s_img = NULL;
lv_obj_t *watchface_btn = NULL;
lv_style_t watchface_app_tile_style;                   /** @brief osm main styte obj */

LV_IMG_DECLARE(swiss_dial_240px);
LV_IMG_DECLARE(swiss_hour_240px);
LV_IMG_DECLARE(swiss_min_240px);
LV_IMG_DECLARE(swiss_sec_240px);
LV_IMG_DECLARE(swiss_hour_s_240px);
LV_IMG_DECLARE(swiss_min_s_240px);
LV_IMG_DECLARE(swiss_sec_s_240px);

void watchface_app_tile_update_task( lv_task_t *task );
bool watchface_powermgm_event_cb( EventBits_t event, void *arg );
bool watchface_touch_event_cb( EventBits_t event, void *arg );
void watchface_avtivate_cb( void );
void watchface_hibernate_cb( void );

void watchface_app_tile_setup( void ) {
    watchface_app_tile_num = mainbar_add_app_tile( 1, 1, "WatchFace Tile" );
    watchface_app_tile = mainbar_get_tile_obj( watchface_app_tile_num );

    lv_style_copy( &watchface_app_tile_style, ws_get_mainbar_style() );
    lv_style_set_radius( &watchface_app_tile_style, LV_OBJ_PART_MAIN, 0 );
    lv_style_set_bg_color( &watchface_app_tile_style, LV_OBJ_PART_MAIN, LV_COLOR_BLACK );

    lv_obj_t *watchface_cont = watchface_app_tile;
/*
    lv_obj_t *watchface_cont = lv_obj_create( watchface_app_tile, NULL );
    lv_obj_set_size( watchface_cont, lv_disp_get_hor_res( NULL ), lv_disp_get_ver_res( NULL ) );
    lv_obj_add_style( watchface_cont, LV_OBJ_PART_MAIN, &watchface_app_tile_style );
    lv_obj_align( watchface_cont, watchface_app_tile, LV_ALIGN_CENTER, 0, 0 );
*/    
    watchface_dial_img = lv_img_create( watchface_cont, NULL );
    lv_obj_set_width( watchface_dial_img, lv_disp_get_hor_res( NULL ) );
    lv_obj_set_height( watchface_dial_img, lv_disp_get_ver_res( NULL ) );
    lv_img_set_src( watchface_dial_img, &swiss_dial_240px );
    lv_obj_align( watchface_dial_img, watchface_cont, LV_ALIGN_CENTER, 0, 0 );

    watchface_hour_s_img = lv_img_create( watchface_cont, NULL );
    lv_img_set_src( watchface_hour_s_img, &swiss_hour_s_240px );
    lv_obj_align( watchface_hour_s_img, watchface_cont, LV_ALIGN_CENTER, 5, 5 );
    lv_img_set_angle( watchface_hour_s_img, 0 );
    watchface_min_s_img = lv_img_create( watchface_cont, NULL );
    lv_img_set_src( watchface_min_s_img, &swiss_min_s_240px );
    lv_obj_align( watchface_min_s_img, watchface_cont, LV_ALIGN_CENTER, 5, 5 );
    lv_img_set_angle( watchface_min_s_img, 0 );
    watchface_sec_s_img = lv_img_create( watchface_cont, NULL );
    lv_img_set_src( watchface_sec_s_img, &swiss_sec_s_240px );
    lv_obj_align( watchface_sec_s_img, watchface_cont, LV_ALIGN_CENTER, 5, 5 );
    lv_img_set_angle( watchface_sec_s_img, 0 );

    watchface_hour_img = lv_img_create( watchface_cont, NULL );
    lv_img_set_src( watchface_hour_img, &swiss_hour_240px );
    lv_obj_align( watchface_hour_img, watchface_cont, LV_ALIGN_CENTER, 0, 0 );
    lv_img_set_angle( watchface_hour_img, 0 );
    watchface_min_img = lv_img_create( watchface_cont, NULL );
    lv_img_set_src( watchface_min_img, &swiss_min_240px );
    lv_obj_align( watchface_min_img, watchface_cont, LV_ALIGN_CENTER, 0, 0 );
    lv_img_set_angle( watchface_hour_img, 0 );
    watchface_sec_img = lv_img_create( watchface_cont, NULL );
    lv_img_set_src( watchface_sec_img, &swiss_sec_240px );
    lv_obj_align( watchface_sec_img, watchface_cont, LV_ALIGN_CENTER, 0, 0 );
    lv_img_set_angle( watchface_hour_img, 0 );

    mainbar_add_tile_activate_cb( watchface_app_tile_num, watchface_avtivate_cb );
    mainbar_add_tile_hibernate_cb( watchface_app_tile_num, watchface_hibernate_cb );

    powermgm_register_cb( POWERMGM_WAKEUP, watchface_powermgm_event_cb, "watchface powermgm" );
    touch_register_cb( TOUCH_UPDATE, watchface_touch_event_cb, "touch watchface" );

    watchface_tile_task = lv_task_create( watchface_app_tile_update_task, 1000, LV_TASK_PRIO_MID, NULL );
    watchface_reload_images();
}

bool watchface_touch_event_cb( EventBits_t event, void *arg ) {
    switch ( event ) {
        case TOUCH_UPDATE: 
            if ( watchface_active ) {
                mainbar_jump_to_maintile( LV_ANIM_OFF );
            } 
            break;
    }
    return( true );
}

void watchface_reload_images( void ) {
    FILE* file;
    /**
     * load dial image
     * 240x240px
     */
    file = fopen( WATCHFACE_DIAL_IMAGE_FILE, "rb" );
    if ( file ) {
        fclose( file );
        lv_img_set_src( watchface_dial_img, WATCHFACE_DIAL_IMAGE_FILE );
        WATCHFACE_LOG("load custom watchface dial from %s", WATCHFACE_DIAL_IMAGE_FILE );
    }
    else {
        lv_img_set_src( watchface_dial_img, &swiss_dial_240px );
        WATCHFACE_LOG("load standard watchface dial");
    }
    lv_obj_align( watchface_dial_img, watchface_app_tile, LV_ALIGN_CENTER, 0, 0 );
    lv_img_cache_invalidate_src( watchface_dial_img );
    /**
     * load hour shadow image
     * 40x240px, center x=20, y=120
     */
    file = fopen( WATCHFACE_HOUR_SHADOW_IMAGE_FILE, "rb" );
    if ( file ) {
        fclose( file );
        lv_img_set_src( watchface_hour_s_img, WATCHFACE_HOUR_SHADOW_IMAGE_FILE );
        WATCHFACE_LOG("load custom watchface hour shadow from %s", WATCHFACE_HOUR_SHADOW_IMAGE_FILE );
    }
    else {
        lv_img_set_src( watchface_hour_s_img, &swiss_hour_s_240px );
        WATCHFACE_LOG("load standard watchface hour shadow");
    }
    lv_obj_align( watchface_hour_s_img, watchface_app_tile, LV_ALIGN_CENTER, 5, 5 );
    lv_img_cache_invalidate_src( watchface_hour_s_img );
    /**
     * load min shadow image
     * 40x240px, center x=20, y=120
     */
    file = fopen( WATCHFACE_MIN_SHADOW_IMAGE_FILE, "rb" );
    if ( file ) {
        fclose( file );
        lv_img_set_src( watchface_min_s_img, WATCHFACE_MIN_SHADOW_IMAGE_FILE );
        WATCHFACE_LOG("load custom watchface min shadow from %s", WATCHFACE_MIN_SHADOW_IMAGE_FILE );
    }
    else {
        lv_img_set_src( watchface_min_s_img, &swiss_min_s_240px );
        WATCHFACE_LOG("load standard watchface min shadow");
    }
    lv_obj_align( watchface_min_s_img, watchface_app_tile, LV_ALIGN_CENTER, 5, 5 );
    lv_img_cache_invalidate_src( watchface_min_s_img );
    /**
     * load sec shadow image
     * 40x240px, center x=20, y=120
     */
    file = fopen( WATCHFACE_SEC_SHADOW_IMAGE_FILE, "rb" );
    if ( file ) {
        fclose( file );
        lv_img_set_src( watchface_sec_s_img, WATCHFACE_SEC_SHADOW_IMAGE_FILE );
        WATCHFACE_LOG("load custom watchface sec shadow from %s", WATCHFACE_SEC_SHADOW_IMAGE_FILE );
    }
    else {
        lv_img_set_src( watchface_sec_s_img, &swiss_sec_s_240px );
        WATCHFACE_LOG("load standard watchface sec shadow");
    }
    lv_obj_align( watchface_sec_s_img, watchface_app_tile, LV_ALIGN_CENTER, 5, 5 );        
    lv_img_cache_invalidate_src( watchface_sec_s_img );
    /**
     * load hour image
     * 40x240px, center x=20, y=120
     */
    file = fopen( WATCHFACE_HOUR_IMAGE_FILE, "rb" );
    if ( file ) {
        fclose( file );
        lv_img_set_src( watchface_hour_img, WATCHFACE_HOUR_IMAGE_FILE );
        WATCHFACE_LOG("load custom watchface hour from %s", WATCHFACE_HOUR_IMAGE_FILE );
    }
    else {
        lv_img_set_src( watchface_hour_img, &swiss_hour_240px );
        WATCHFACE_LOG("load standard watchface hour");
    }
    lv_obj_align( watchface_hour_img, watchface_app_tile, LV_ALIGN_CENTER, 0, 0 );        
    lv_img_cache_invalidate_src( watchface_hour_img );
    /**
     * load min image
     * 40x240px, center x=20, y=120
     */
    file = fopen( WATCHFACE_MIN_IMAGE_FILE, "rb" );
    if ( file ) {
        fclose( file );
        lv_img_set_src( watchface_min_img, WATCHFACE_MIN_IMAGE_FILE );
        WATCHFACE_LOG("load custom watchface min from %s", WATCHFACE_MIN_IMAGE_FILE );
    }
    else {
        lv_img_set_src( watchface_min_img, &swiss_min_240px );
        WATCHFACE_LOG("load standard watchface min");
    }
    lv_obj_align( watchface_min_img, watchface_app_tile, LV_ALIGN_CENTER, 0, 0 );        
    lv_img_cache_invalidate_src( watchface_min_img );
    /**
     * load sec image
     * 40x240px, center x=20, y=120
     */
    file = fopen( WATCHFACE_SEC_IMAGE_FILE, "rb" );
    if ( file ) {
        fclose( file );
        lv_img_set_src( watchface_sec_img, WATCHFACE_SEC_IMAGE_FILE );
        WATCHFACE_LOG("load custom watchface sec from %s", WATCHFACE_SEC_IMAGE_FILE );
    }
    else {
        lv_img_set_src( watchface_sec_img, &swiss_sec_240px );
        WATCHFACE_LOG("load standard watchface sec");
    }
    lv_obj_align( watchface_sec_img, watchface_app_tile, LV_ALIGN_CENTER, 0, 0 );        
    lv_img_cache_invalidate_src( watchface_sec_img );
}

bool watchface_powermgm_event_cb( EventBits_t event, void *arg ) {
    switch ( event ) {
        case POWERMGM_WAKEUP:
            if ( !display_get_block_return_maintile() ) {
                if ( watchface_enable_after_wakeup ) {
                    mainbar_jump_to_tilenumber( watchface_app_tile_num, LV_ANIM_OFF );
                }
            }
    }
    return( true );
}

void watchface_avtivate_cb( void ) {
    watchface_active = true;
    statusbar_hide( true );
    powermgm_set_perf_mode();
}

void watchface_hibernate_cb( void ) {
    watchface_active = false;
    statusbar_hide( false );
    powermgm_set_normal_mode();
}

void watchface_app_tile_update_task( lv_task_t *task ) {
	if ( watchface_active ) {
        tm info;
        time_t now;
        time(&now);
        localtime_r( &now, &info );

        //Angle calculation for Hands
        int Angle_S = (int)((info.tm_sec % 60) * 60 );
        int Angle_M = (int)((info.tm_min % 60) * 60 ) + (int)(info.tm_sec % 60);
        int Angle_H = (int)((info.tm_hour % 12 ) * 300 ) + (int)((info.tm_min % 60 ) * 5);

        while (Angle_S >= 3600)
            Angle_S = Angle_S - 3600;
        while (Angle_M >= 3600)
            Angle_M = Angle_M - 3600;
        while (Angle_H >= 3600)
            Angle_H = Angle_H - 3600;

        lv_img_set_angle( watchface_hour_img, Angle_H );
        lv_img_set_angle( watchface_min_img, Angle_M );
        lv_img_set_angle( watchface_sec_img, Angle_S );

        lv_img_set_angle( watchface_hour_s_img, Angle_H );
        lv_img_set_angle( watchface_min_s_img, Angle_M );
        lv_img_set_angle( watchface_sec_s_img, Angle_S );
    }
}

void watchface_enable_tile_after_wakeup( bool enable ) {
    watchface_enable_after_wakeup = enable;
}