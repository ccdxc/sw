import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { MonitoringGroupComponent } from './monitoring-group.component';
import { MonitoringRoutingModule } from './monitoring-group.route';
import { SyslogComponent } from './syslog/syslog.component';
import { MatchruleComponent } from './matchrule/matchrule.component';
import { SharedModule } from '../shared/shared.module';
import { NetworkgraphComponent } from './mirrorsessions/networkgraph/networkgraph.component';




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
  declarations: [ MonitoringGroupComponent, SyslogComponent, MatchruleComponent, NetworkgraphComponent],
  exports: [ MonitoringGroupComponent, SyslogComponent, MatchruleComponent]
})
export class MonitoringGroupModule { }
