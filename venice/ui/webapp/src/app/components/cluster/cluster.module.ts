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

import { ClusterRoutingModule } from './cluster.route';

import { ClusterComponent } from './cluster.component';
import { NaplesComponent } from './naples/naples.component';



@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    FlexLayoutModule,
    PrimengModule,
    MaterialdesignModule,
    WidgetsModule,
    SharedModule,

    ClusterRoutingModule
  ],
  declarations: [
    ClusterComponent,
    NaplesComponent,
  ],
  entryComponents: [
    NaplesComponent,
    ClusterComponent,
  ],
})
export class ClusterModule { }
