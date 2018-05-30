import { NgModule } from '@angular/core';
import { StoreModule } from '@ngrx/store';
import { EffectsModule } from '@ngrx/effects';

import { settingsReducer } from './settings.reducer';
import { SettingsEffects } from './settings.effects';
import { SettingsComponent } from './settings/settings.component';


import { MaterialdesignModule } from '@lib/materialdesign.module';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
@NgModule({
  imports: [
    CommonModule,
    FormsModule,

    MaterialdesignModule,

    StoreModule.forFeature('settings', settingsReducer),
    EffectsModule.forFeature([SettingsEffects])
  ],
  declarations: [SettingsComponent]
})
export class SettingsModule { }
