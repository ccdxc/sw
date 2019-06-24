import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule } from '@angular/forms';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { AdminComponent } from './admin.component';
import { AdminRoutingModule } from './admin.route';
@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    PrimengModule,
    MaterialdesignModule,
    FlexLayoutModule,
    FormsModule,
    SharedModule,
    AdminRoutingModule
  ],
  declarations: [AdminComponent],
  exports: [AdminComponent],
  entryComponents: [
    AdminComponent
  ],
})
export class AdminModule { }
