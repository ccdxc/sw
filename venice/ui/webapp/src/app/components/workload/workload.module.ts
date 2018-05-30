/**-----
 angular JS -  imports
 ------------------*/
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Routes, RouterModule } from '@angular/router';
/**-----
 Thrid-party -  imports
 ------------------*/
import { MomentModule } from 'angular2-moment';
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { FlexLayoutModule } from '@angular/flex-layout';
import { GridsterModule } from 'angular-gridster2';
/**-----
 Venice Framework UI -  imports
 ------------------*/
import { WidgetsModule } from 'web-app-framework';
/**-----
 Venice UI -  imports
 ------------------*/
import { WorkloadComponent } from './workload.component';
import { workloadRouter } from './workload.router';
import { WorkloadwidgetComponent } from './workloadwidget/workloadwidget.component';
import { WorkloadModalComponent } from './workloadmodal/workloadmodal.component';
import { SharedModule } from '@app/components/shared//shared.module';
import { LabelselectorComponent } from '@app/components/workload/labelselector/labelselector.component';


@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,

    MomentModule,
    GridsterModule,
    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,

    WidgetsModule,

    workloadRouter,
    SharedModule

  ],
  declarations: [WorkloadComponent, WorkloadwidgetComponent, WorkloadModalComponent, LabelselectorComponent],
  exports: [WorkloadComponent, WorkloadwidgetComponent],
  entryComponents: [
    WorkloadwidgetComponent,
    WorkloadModalComponent
  ],
})
export class WorkloadModule { }
