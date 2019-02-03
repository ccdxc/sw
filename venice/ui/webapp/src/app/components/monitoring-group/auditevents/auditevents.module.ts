import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

/**-----
 Venice UI -  imports
 ------------------*/
 import { SharedModule } from '@app/components/shared/shared.module';
 import { MaterialdesignModule } from '@lib/materialdesign.module';
 import { PrimengModule } from '@lib/primeng.module';
 import { WidgetsModule } from 'web-app-framework';

import { AuditeventsComponent } from './auditevents.component';
import { AuditeventsRoutingModule } from './auditevents.route';
@NgModule({
  declarations: [
    AuditeventsComponent
  ],
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,
    SharedModule,

    AuditeventsRoutingModule
  ]
})
export class AuditeventsModule { }
