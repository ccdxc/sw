import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { FormControl } from '@angular/forms';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { LDAPCheckResponse, LDAPCheckType } from '@app/components/settings-group/authpolicy/.';
import { AuthPolicyUtil } from '@app/components/settings-group/authpolicy/AuthPolicyUtil';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { AuthAuthenticationPolicy, AuthLdap, IApiStatus, IAuthAuthenticationPolicy } from '@sdk/v1/models/generated/auth';
import { MessageService } from 'primeng/primeng';
import { Observable } from 'rxjs/Observable';


/**
 * AuthpolicyComponent allow user to manage authentication policy.
 * When Venice is setup, there must be day-0 opertion to create auth-policy so that user can login at first place.
 * Thus, the component manage update/test operations.
 *
 * This component internally contain  local, ldap, radius sub components.  It handles REST call on behalf of sub-component. For example, it manages LDAP component's test-server calls.
 */
@Component({
  selector: 'app-authpolicy',
  templateUrl: './authpolicy.component.html',
  styleUrls: ['./authpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class AuthpolicyComponent extends BaseComponent implements OnInit {
  secretFormControl: FormControl = new FormControl('', []);
  enableUpdateSecretButton: boolean = false;
  authOrder = ['LOCAL', 'LDAP', 'RADIUS'];
  authPolicy: AuthAuthenticationPolicy = new AuthAuthenticationPolicy({ spec: { authenticators: { 'authenticator-order': ['LOCAL', 'LDAP'] } } });

  // these variables will pass to LDAPComponent
  _ldapConnCheckResponse: LDAPCheckResponse = null;
  _ldapBindCheckResponse: LDAPCheckResponse = null;

  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService,
    protected messageService: MessageService
  ) {
    super(_controllerService, messageService);
  }

  ngOnInit() {
    // Setting the toolbar
    this._controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Settings', url: '' }, { label: 'Auth Policy', url: '' }]
    });
    // Retrieve auth-policy
    this.getAuthenticationPolicy();
  }


  getAuthenticationPolicy() {
    this._authService.GetAuthenticationPolicy().subscribe(
      response => {
        const body = response.body;
        this.authPolicy = new AuthAuthenticationPolicy(body);
      },
      this.restErrorHandler('Failed to get Authentication Policy')
    );
  }

  swapRanks(newRank, oldRank) {
    // Since ranks should only ever be moving one up or one down
    // We swap the items
    const authOrder = this.authPolicy.spec.authenticators['authenticator-order'];
    const copy = Utility.getLodash().cloneDeep(authOrder);
    const temp = copy[newRank];
    copy[newRank] = copy[oldRank];
    copy[oldRank] = temp;
    this.authPolicy.spec.authenticators['authenticator-order'] = copy;
  }

  getClassName(): string {
    return this.constructor.name;
  }

  onCheckLDAPServerConnect(ldap: AuthLdap) {
    this._authService.LdapConnectionCheck(this.authPolicy.getFormGroupValues()).subscribe(
      response => {
        const respAuthPolicy: AuthAuthenticationPolicy = response.body as AuthAuthenticationPolicy;
        const ldapCheckResponse = this.makeLDAPCheckResponse(LDAPCheckType.CONNECTION, ldap, respAuthPolicy);
        const connCheckResponseError = AuthPolicyUtil.processLDAPCheckResponse(ldapCheckResponse);
        if (connCheckResponseError.errors.length > 0) {
          this._ldapConnCheckResponse = ldapCheckResponse;
        } else {
          this.handleLDAPServerCheckSuccess(LDAPCheckType.CONNECTION);
        }
      },
      this.restErrorHandler('Failed to check LDAP server configuration')
    );
  }

  handleLDAPServerCheckSuccess(type: LDAPCheckType) {
    alert('LDAP ' + type + ' pass'); // TODO: use toaster later.
  }

  onCheckLDAPBindConnect(ldap: AuthLdap) {
    this._authService.LdapBindCheck(this.authPolicy.getFormGroupValues()).subscribe(
      response => {
        const respAuthPolicy: AuthAuthenticationPolicy = response.body as AuthAuthenticationPolicy;
        const ldapCheckResponse = this.makeLDAPCheckResponse(LDAPCheckType.BIND, ldap, respAuthPolicy);
        const ldapBindCheckResponseError = AuthPolicyUtil.processLDAPCheckResponse(ldapCheckResponse);
        if (ldapBindCheckResponseError.errors.length > 0) {
          this._ldapBindCheckResponse = ldapCheckResponse;
        } else {
          this.handleLDAPServerCheckSuccess(LDAPCheckType.BIND);
        }
      },
      this.restErrorHandler('Failed to check LDAP server configuration')
    );
  }

  makeLDAPCheckResponse(type: LDAPCheckType, checkedLdap: AuthLdap, responseAuthPolicy: AuthAuthenticationPolicy): LDAPCheckResponse {
    const ldapCheckResponse = {} as LDAPCheckResponse;
    ldapCheckResponse.type = type;
    ldapCheckResponse.authpolicy = responseAuthPolicy;
    return ldapCheckResponse;
  }

  /**
   * This API serves html template
   * @param isNewLDAP
   *
   * Even through we passes in "isNewLDAP" parameter, (isNewLDAP is for creating auth-policy), it is not likely isNewLDAP will be true.
   * If the user is in the UI, they must have setup at least a local auth policy. We should only be updating the current one.
   */
  onInvokeSaveLDAP(isNewLDAP: boolean) {
    this.saveAuthenticationPolicy();
  }

  saveAuthenticationPolicy() {
    let handler: Observable<{ body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number }>;
    // If the user is in the UI, they must have setup at least a local auth policy. We should only be updating the current one.
    handler = this._authService.UpdateAuthenticationPolicy(this.authPolicy.getFormGroupValues());
    handler.subscribe(
      (response) => {
        this.invokeSuccessToaster('Update Successful', 'Updated Authentication policy');
        const status = response.statusCode;
        const body = response.body;
        this.authPolicy = new AuthAuthenticationPolicy(body);
      },
      this.restErrorHandler('Update Failed')
    );
  }
}
