import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Routes, RouterModule } from '@angular/router';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';

import { MonitoringComponent } from './monitoring.component';
import { TroubleshootingComponent } from './troubleshooting/troubleshooting.component';
import { MonitoringRoutingModule } from './monitoring.route';
import { SharedModule } from '@app/components/shared/shared.module';
import { FlexLayoutModule } from '@angular/flex-layout';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,
    SharedModule,
    FlexLayoutModule,

    PrimengModule,
    MaterialdesignModule,
    MonitoringRoutingModule
  ],
  declarations: [TroubleshootingComponent, MonitoringComponent],
  exports: [TroubleshootingComponent, MonitoringComponent]
})
export class MonitoringModule { }
