import {NgModule} from '@angular/core';
import {CommonModule} from '@angular/common';
import {FormsModule, ReactiveFormsModule, } from '@angular/forms';
import {PrimengModule} from '@lib/primeng.module';
import {FlexLayoutModule} from '@angular/flex-layout';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {DSCProfileRoutingModule} from '@components/cluster-group/dscprofiles/dscprofiles.route';
import {SharedModule} from '@components/shared/shared.module';
import {DscprofilesComponent} from '@app/components/cluster-group/dscprofiles/dscprofiles.component';
import {NewdscprofileComponent } from '@app/components/cluster-group/dscprofiles/newdscprofile/newdscprofile.component';

import { WidgetsModule } from 'web-app-framework';

@NgModule(
  {
    imports: [
      CommonModule,
      FormsModule,

      ReactiveFormsModule,
      PrimengModule,
      FlexLayoutModule,
      MaterialdesignModule,
      WidgetsModule,
      DSCProfileRoutingModule,
      SharedModule
    ],
    declarations: [DscprofilesComponent, NewdscprofileComponent]
  }
)

export class DscprofilesModule { }
