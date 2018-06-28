import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule } from '@angular/forms';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { MonitoringGroupComponent } from './monitoring-group.component';
import { MonitoringRoutingModule } from './monitoring-group.route';
import { TroubleshootingComponent } from './troubleshooting/troubleshooting.component';




@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    SharedModule,
    FlexLayoutModule,

    PrimengModule,
    MaterialdesignModule,
    MonitoringRoutingModule
  ],
  declarations: [TroubleshootingComponent, MonitoringGroupComponent],
  exports: [TroubleshootingComponent, MonitoringGroupComponent]
})
export class MonitoringGroupModule { }
