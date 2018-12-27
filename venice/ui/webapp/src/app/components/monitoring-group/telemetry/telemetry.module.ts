import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
/**-----
 Venice UI -  imports
 ------------------*/
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { TelemetryComponent } from './telemetry.component';
import { FwlogpoliciesComponent } from './fwlogpolicies/fwlogpolicies.component';
import { TelemetryRoutingModule } from './telemetry.route';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { WidgetsModule } from 'web-app-framework';
import { FlowexportComponent } from './flowexport/flowexport.component';
import { NewflowexportComponent } from './flowexport/newflowexport/newflowexport.component';
import { NewfwlogpolicyComponent } from './fwlogpolicies/newfwlogpolicy/newfwlogpolicy.component';
import { MonitoringGroupModule } from '../monitoring-group.module';



@NgModule({
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,


    TelemetryRoutingModule,
    SharedModule,
    MonitoringGroupModule
  ],
  declarations: [
    TelemetryComponent,
    FwlogpoliciesComponent,
    FlowexportComponent,
    NewflowexportComponent,
    NewfwlogpolicyComponent,
  ]
})
export class TelemetryModule { }
