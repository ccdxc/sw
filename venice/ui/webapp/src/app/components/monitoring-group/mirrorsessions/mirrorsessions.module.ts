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

 import { MirrorsessionsRoutingModule } from './mirrorsessions.route';
import { MirrorsessionsComponent } from './mirrorsessions.component';
import { NewmirrorsessionComponent } from './newmirrorsession/newmirrorsession.component';

@NgModule({
  declarations: [MirrorsessionsComponent, NewmirrorsessionComponent],
  imports: [
    CommonModule,
    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,
    SharedModule,
    MirrorsessionsRoutingModule
  ]
})
export class MirrorsessionsModule { }
