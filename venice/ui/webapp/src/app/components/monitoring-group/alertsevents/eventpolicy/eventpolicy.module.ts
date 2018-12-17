import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
/**-----
 Venice UI -  imports
 ------------------*/
import { SharedModule } from '@app/components/shared//shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';
import { EventpolicyComponent } from './eventpolicy.component';
import { EventpolicyRoutingModule } from './eventpolicy.route';
import { NeweventpolicyComponent } from './neweventpolicy/neweventpolicy.component';
import { MonitoringGroupModule } from '../../monitoring-group.module';


@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,

    EventpolicyRoutingModule,
    SharedModule,
    WidgetsModule,
    SharedModule,
    MonitoringGroupModule
  ],
  declarations: [EventpolicyComponent, NeweventpolicyComponent],
})
export class EventPolicyModule { }
