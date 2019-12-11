import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import { UpdateclusterTLSRoutingModule } from './updatecluster-tls.route';
import { ClusterModule } from '@app/components/cluster-group/cluster/cluster.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { FlexLayoutModule } from '@angular/flex-layout';

import { UpdateclustertlsComponent } from './updateclustertls/updateclustertls.component';
import { UpdateclusterTlsComponent } from './updatecluster-tls.component';
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
@NgModule({
  declarations: [UpdateclusterTlsComponent, UpdateclustertlsComponent],
  imports: [
    CommonModule,
    UpdateclusterTLSRoutingModule,
    ClusterModule,
    SharedModule,
    FormsModule,
    FlexLayoutModule,
    PrimengModule,
    ReactiveFormsModule,
    MaterialdesignModule,
  ]
})
export class UpdateclusterTLSModule { }
