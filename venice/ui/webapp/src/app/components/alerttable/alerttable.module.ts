import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Routes, RouterModule } from '@angular/router';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { MomentModule } from 'angular2-moment';

import { AlerttableComponent } from './alerttable.component';
import { alerttableRouter } from './alerttable.route';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    MaterialdesignModule,
    MomentModule,

    alerttableRouter
  ],
  declarations: [AlerttableComponent],
  exports: [AlerttableComponent]
})
export class AlerttableModule { }
