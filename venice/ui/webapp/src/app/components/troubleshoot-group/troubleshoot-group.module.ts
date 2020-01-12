import { WidgetsModule } from 'web-app-framework';
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { TroubleshootGroupComponent } from './troubleshoot-group.component';
import { TroubleshootRoutingModule } from './troubleshoot-group.route';
import { MatchruleComponent } from '../troubleshoot-group/matchrule/matchrule.component';
import { NetworkgraphComponent } from './mirrorsessions/networkgraph/networkgraph.component';
import { SharedModule } from '../shared/shared.module';


@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,
    FlexLayoutModule,
    WidgetsModule,
    PrimengModule,
    MaterialdesignModule,
    TroubleshootRoutingModule,
    SharedModule
  ],
  declarations: [ TroubleshootGroupComponent, MatchruleComponent, NetworkgraphComponent],
  exports: [ TroubleshootGroupComponent, MatchruleComponent, NetworkgraphComponent]
})
export class TroubleshootGroupModule { }
