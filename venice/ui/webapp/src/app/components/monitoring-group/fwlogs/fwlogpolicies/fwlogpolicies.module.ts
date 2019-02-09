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
import { FwlogpoliciesRoutingModule } from './fwlogpolicies.route';
import { MonitoringGroupModule } from '../../monitoring-group.module';
import { NewfwlogpolicyComponent } from './newfwlogpolicy/newfwlogpolicy.component';
import { FwlogpoliciesComponent } from './fwlogpolicies.component';



@NgModule({
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,


    FwlogpoliciesRoutingModule,
    SharedModule,
    MonitoringGroupModule
  ],
  declarations: [
    FwlogpoliciesComponent,
    NewfwlogpolicyComponent,
  ]
})
export class FwlogpoliciesModule { }
