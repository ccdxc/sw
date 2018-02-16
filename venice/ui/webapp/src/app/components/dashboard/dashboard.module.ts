import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Routes, RouterModule } from '@angular/router';
import { GridsterModule } from 'angular2gridster';
import { PrimengModule } from '@lib/primeng.module';
/**-----
 Venice Framework UI -  imports
 ------------------*/
 import {WidgetsModule} from 'web-app-framework';

import {DashboardComponent} from './dashboard.component';

import {dashboardRouter} from './dashboard.router';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    GridsterModule,
    WidgetsModule,

    dashboardRouter
  ],
  declarations: [DashboardComponent]
})
export class DashboardModule { }
