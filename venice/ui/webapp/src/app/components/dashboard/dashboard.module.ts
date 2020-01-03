import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Routes, RouterModule } from '@angular/router';
// import { GridsterModule } from 'angular2gridster';
import { GridsterModule } from 'angular-gridster2';
import { PrimengModule } from '@lib/primeng.module';
import { FlexLayoutModule } from '@angular/flex-layout';
import { MaterialdesignModule } from '@lib/materialdesign.module';
/**-----
 Venice Framework UI -  imports
 ------------------*/
import { WidgetsModule } from 'web-app-framework';

import { DashboardComponent } from './dashboard.component';
import { SystemcapacitywidgetComponent } from './widgets/systemcapacity/systemcapacity.component';
import { dashboardRouter } from './dashboard.router';
import { DsbdworkloadComponent } from './widgets/dsbdworkload/dsbdworkload.component';
import { ForwardDropComponent } from './widgets/forwarddrop/forwarddrop.component';
import { PolicyhealthComponent } from './widgets/policyhealth/policyhealth.component';
import { SoftwareversionComponent } from './widgets/softwareversion/softwareversion.component';
import { NaplesComponent } from './widgets/naples/naples.component';
import { SharedModule } from '@app/components/shared/shared.module';
import { WorkloadsComponent } from './workloads/workloads.component';

@NgModule({
  id: 'dashboardModule',
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    GridsterModule,
    WidgetsModule,
    FlexLayoutModule,
    MaterialdesignModule,

    dashboardRouter,
    SharedModule
  ],
  declarations: [DashboardComponent, SystemcapacitywidgetComponent, DsbdworkloadComponent, ForwardDropComponent, PolicyhealthComponent, SoftwareversionComponent, NaplesComponent, WorkloadsComponent]
})
export class DashboardModule { }
