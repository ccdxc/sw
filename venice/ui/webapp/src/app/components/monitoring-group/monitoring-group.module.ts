import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { MonitoringGroupComponent } from './monitoring-group.component';
import { MonitoringRoutingModule } from './monitoring-group.route';
import { SharedModule } from '../shared/shared.module';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,
    FlexLayoutModule,

    PrimengModule,
    MaterialdesignModule,
    MonitoringRoutingModule,
    SharedModule
  ],
  declarations: [ MonitoringGroupComponent],
  exports: [ MonitoringGroupComponent]
})
export class MonitoringGroupModule { }
