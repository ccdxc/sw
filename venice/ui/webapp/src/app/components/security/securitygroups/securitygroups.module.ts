import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';

import { SecuritygroupsComponent } from '@app/components/security/securitygroups/securitygroups.component';
import { NewsecuritygroupComponent } from '@app/components/security/securitygroups/newsecuritygroup/newsecuritygroup.component';

import { SecuritygroupsRoutingModule } from '@app/components/security/securitygroups/securitygroups.route';


@NgModule({
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

    SecuritygroupsRoutingModule
  ],
  declarations: [SecuritygroupsComponent, NewsecuritygroupComponent],
})
export class SecuritygroupsModule { }
