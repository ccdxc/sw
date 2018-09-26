import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Routes, RouterModule } from '@angular/router';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { FlexLayoutModule } from '@angular/flex-layout';

import { SharedModule } from '@app/components/shared//shared.module';
/**-----
 Venice Framework UI -  imports
 ------------------*/
import { WidgetsModule } from 'web-app-framework';

import { SettingsGroupRoutingModule } from './settings-group.route';

import { SettingsGroupComponent } from './settings-group.component';
import { SettingsComponent } from '@app/components/settings-group/settings/settings.component';
import { StoreModule } from '@ngrx/store';
import { EffectsModule } from '@ngrx/effects';
import { settingsReducer } from '@app/components/settings-group/settings.reducer';
import { SettingsEffects } from '@app/components/settings-group/settings.effects';



@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    FlexLayoutModule,
    PrimengModule,
    MaterialdesignModule,
    WidgetsModule,
    SharedModule,
    StoreModule.forFeature('settings', settingsReducer),
    EffectsModule.forFeature([SettingsEffects]),

    SettingsGroupRoutingModule
  ],
  declarations: [
    SettingsGroupComponent,
    SettingsComponent,
  ],
  entryComponents: [
    SettingsGroupComponent,
    SettingsComponent,
  ],
})
export class SettingsGroupModule { }
