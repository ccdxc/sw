import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
/**-----
 Venice UI -  imports
 ------------------*/
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { WidgetsModule } from 'web-app-framework';
import { MonitoringGroupModule } from '../monitoring-group.module';
import { FwlogsRoutingModule } from './fwlogs.route';
import { FwlogsComponent } from './fwlogs.component';



@NgModule({
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,


    FwlogsRoutingModule,
    SharedModule,
    MonitoringGroupModule
  ],
  declarations: [
    FwlogsComponent,
  ]
})
export class FwlogsModule { }
