import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { SecurityComponent } from './security.component';
import { SecurityRoutingModule } from './security.route';
@NgModule({
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    MaterialdesignModule,
    FlexLayoutModule,
    FormsModule,
    ReactiveFormsModule,
    SharedModule,
    SecurityRoutingModule
  ],
  declarations: [SecurityComponent],
  exports: [SecurityComponent],
  entryComponents: [
    SecurityComponent
  ],
})
export class SecurityModule { }
