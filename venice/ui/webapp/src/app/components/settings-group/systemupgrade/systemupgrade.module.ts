import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

/**-----
 Venice UI lib-  imports
 ------------------*/
import { SharedModule } from '@app/components/shared//shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';

/**-----
 Venice UI local module-  imports
 ------------------*/
import { SystemupgradeRoutingModule } from './systemupgrade.route';

import { SystemupgradeComponent } from './systemupgrade.component';

@NgModule({
  declarations: [SystemupgradeComponent],
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    FormsModule,
    ReactiveFormsModule,

    SystemupgradeRoutingModule,
    SharedModule
  ]
})
export class SystemupgradeModule { }
