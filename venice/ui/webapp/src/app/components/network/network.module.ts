import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Routes, RouterModule } from '@angular/router';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';


import { NetworkComponent } from './network/network.component';
import { NetworkcoreComponent } from './networkcore.component';

import {NetworkRoutingModule } from './network.route';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    MaterialdesignModule,

    NetworkRoutingModule
  ],
  declarations: [NetworkComponent, NetworkcoreComponent],
  entryComponents: [
    NetworkcoreComponent
  ]
})
export class NetworkModule { }
