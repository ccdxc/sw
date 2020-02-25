import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Routes, RouterModule } from '@angular/router';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { FlexLayoutModule } from '@angular/flex-layout';

import { SharedModule } from '@app/components/shared//shared.module';
/**-----
 Venice Framework UI -  imports
 ------------------*/
import { WidgetsModule } from 'web-app-framework';

import { ControllerRoutingModule } from './controller.route';

import { ControllerComponent } from './controller.component';



@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    FlexLayoutModule,
    PrimengModule,
    MaterialdesignModule,
    WidgetsModule,
    SharedModule,
    ControllerRoutingModule
  ],
  declarations: [
    ControllerComponent,
  ],
})
export class ControllerModule { }
