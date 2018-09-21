import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { SgpoliciesComponent } from './sgpolicies.component';
import { SgpolicydetailComponent } from './sgpolicydetail/sgpolicydetail.component';
import { SgpoliciesRoutingModule } from '@app/components/security/sgpolicies/sgpolicies.route';


@NgModule({
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

    SgpoliciesRoutingModule
  ],
  declarations: [SgpoliciesComponent, SgpolicydetailComponent],
})
export class SgpoliciesModule { }
