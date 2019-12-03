import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
/**-----
 Venice UI -  imports
 ------------------*/
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { FlowexportComponent } from './flowexport.component';
import { FlowexportRoutingModule } from './flowexport.route';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { WidgetsModule } from 'web-app-framework';
import { FlowexportpolicyComponent } from './flowexportpolicy/flowexportpolicy.component';
import { NewflowexportpolicyComponent } from './flowexportpolicy/newflowexportpolicy/newflowexportpolicy.component';
import { TroubleshootGroupModule } from '../troubleshoot-group.module';



@NgModule({
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,


    FlowexportRoutingModule,
    SharedModule,
    TroubleshootGroupModule
  ],
  declarations: [
    FlowexportComponent,
    FlowexportpolicyComponent,
    NewflowexportpolicyComponent,
  ]
})
export class FlowexportModule { }
