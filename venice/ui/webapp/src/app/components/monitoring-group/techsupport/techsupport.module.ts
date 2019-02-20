import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

/**-----
 Venice UI -  imports
 ------------------*/
 import { SharedModule } from '@app/components/shared/shared.module';
 import { MaterialdesignModule } from '@lib/materialdesign.module';
 import { PrimengModule } from '@lib/primeng.module';
 import { WidgetsModule } from 'web-app-framework';
import { TechsupportComponent } from './techsupport.component';

import { TechsupportRoutingModule } from './techsupport.route';
import { NewtechsupportComponent } from './newtechsupport/newtechsupport.component';

@NgModule({
  declarations: [TechsupportComponent, NewtechsupportComponent],
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,
    SharedModule,
    TechsupportRoutingModule
  ]
})
export class TechsupportModule { }
