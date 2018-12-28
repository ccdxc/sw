import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';

import { SecurityappsComponent } from './securityapps.component';
import { SecurityappsRoutingModule } from './securityapps.route';

@NgModule({
  declarations: [SecurityappsComponent],
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    MaterialdesignModule,
    SharedModule,
    FlexLayoutModule,
    FormsModule,
    ReactiveFormsModule,
    SharedModule,

    SecurityappsRoutingModule
  ]
})
export class SecurityappsModule { }
