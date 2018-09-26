import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
/**-----
 Venice UI -  imports
 ------------------*/
import { SharedModule } from '@app/components/shared//shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { AuthpolicyComponent } from '@app/components/settings-group/authpolicy/authpolicy.component';
import { AuthPolicyRoutingModule } from '@app/components/settings-group/authpolicy/authpolicy.route';
import { LdapComponent } from './ldap/ldap.component';
import { LocalComponent } from './local/local.component';
import { AuthpolicybaseComponent } from './authpolicybase/authpolicybase.component';
import { RadiusComponent } from './radius/radius.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';



@NgModule({
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    FormsModule,
    ReactiveFormsModule,

    AuthPolicyRoutingModule,
    SharedModule
  ],
  declarations: [AuthpolicyComponent, LdapComponent, LocalComponent, AuthpolicybaseComponent, RadiusComponent]
})
export class AuthpolicyModule { }
