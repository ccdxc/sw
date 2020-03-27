import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { WidgetsModule } from 'web-app-framework';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';

import { FirewallprofilesComponent } from './firewallprofiles.component';
import { NewfirewallprofileComponent } from './newfirewallprofile/newfirewallprofile.component';


import { FirewallprofilesRoutingModule } from './firewallprofiles.route';
@NgModule({
  declarations: [FirewallprofilesComponent, NewfirewallprofileComponent],
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    MaterialdesignModule,
    WidgetsModule,
    SharedModule,
    FlexLayoutModule,
    FormsModule,
    ReactiveFormsModule,
    SharedModule,

    FirewallprofilesRoutingModule
  ]
})
export class FirewallprofilesModule { }
