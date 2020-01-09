import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

import { ClusterModule } from '@app/components/cluster-group/cluster/cluster.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { UpdateclusterTlsComponent } from './updatecluster-tls.component';
import { UpdateclusterTLSRoutingModule } from './updatecluster-tls.route';

@NgModule({
  declarations: [UpdateclusterTlsComponent],
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
