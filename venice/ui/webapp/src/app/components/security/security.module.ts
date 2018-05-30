import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { SecurityComponent } from './security.component';
import { SecurityRoutingModule } from './security.route';
import { SgpolicyComponent } from './sgpolicy/sgpolicy.component';


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

    SecurityRoutingModule
  ],
  declarations: [SgpolicyComponent, SecurityComponent],
  exports: [SgpolicyComponent, SecurityComponent],
  entryComponents: [
    SgpolicyComponent,
    SecurityComponent
  ],
})
export class SecurityModule { }
