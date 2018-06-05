import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';

import { ClusterComponent } from './cluster.component';
import { ClusterRoutingModule } from './cluster.route';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    MaterialdesignModule,

    ClusterRoutingModule
  ],
  declarations: [ClusterComponent]
})
export class ClusterModule { }