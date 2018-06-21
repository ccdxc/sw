import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { FlexLayoutModule } from '@angular/flex-layout';

/**-----
 Venice UI -  imports
 ------------------*/
import { ClusterComponent } from './cluster.component';
import { ClusterRoutingModule } from './cluster.route';
import { SharedModule } from '@app/components/shared//shared.module';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,

    ClusterRoutingModule,
    SharedModule
  ],
  declarations: [ClusterComponent]
})
export class ClusterModule { }
