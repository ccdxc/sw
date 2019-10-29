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


import { TroubleshootingComponent } from './troubleshooting.component';
import { NetworkgraphComponent } from './networkgraph/networkgraph.component';
import { TroubleshootingRoutingModule } from './troubleshooting.route';
@NgModule({
  declarations: [TroubleshootingComponent, NetworkgraphComponent],
  imports: [
    CommonModule,
    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,
    SharedModule,

    TroubleshootingRoutingModule
  ]
})
export class TroubleshootingModule { }
