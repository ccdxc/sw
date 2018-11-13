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
import { Observable } from 'rxjs';
import { Eventtypes } from '@app/enum/eventtypes.enum';


/**
 * AuthpolicyComponent allow user to manage authentication policy.
 * When Venice is setup, there must be day-0 opertion to create auth-policy so that user can login at first place.
 * Thus, the component manage update/test operations.
 *
 * This component internally contain  local, ldap, radius sub components.  It handles REST call on behalf of sub-component. For example, it manages LDAP component's test-server calls.
 *
 * As Auth-policy is a singleton in Venice, user should re-login to Venice after auth-policy is updated.
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
    this.setupToolbarItems();
    // Retrieve auth-policy
    this.getAuthenticationPolicy();
  }

  /**
   * Since AuthPolicy is a singleton in Venice, we don't have to use watch-api to monitor AuthPolicy, we just let user refresh data.
   *
   * There are two places we will call this API.
   * 1. Component is initialized (show only Refresh button)
   * 2. User chagne policy rank
   * If user changes the policy rank, we will display a "save" button.
   */
  private setupToolbarItems(rankChanged: boolean = false) {
    const buttons = [
      {
        cssClass: 'global-button-primary authpolicy-toolbar-button',
        text: 'Refresh',
        callback: () => {
          this.getAuthenticationPolicy();
        }
      }
    ];
    if (rankChanged === true) {
      const saveRankChange = {
        cssClass: 'global-button-primary authpolicy-toolbar-button',
        text: 'Save',
        callback: () => {
          this.saveAuthenticationPolicy();
        }
      };
      buttons.push(saveRankChange);
    }

    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Settings', url: '' }, { label: 'Auth Policy', url: '' }]
    });
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
    this.setupToolbarItems(true);
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
          this.handleLDAPServerCheckFailure(LDAPCheckType.CONNECTION);
        } else {
          this.handleLDAPServerCheckSuccess(LDAPCheckType.CONNECTION);
        }
      },
      this.restErrorHandler('Failed to check LDAP server configuration')
    );
  }

  handleLDAPServerCheckSuccess(type: LDAPCheckType) {
    this.invokeSuccessToaster('Test LDAP ' + type + ' pass', '');
  }

  handleLDAPServerCheckFailure(type: LDAPCheckType) {
    this.invokeRESTErrorToaster('Test LDAP ' + type + ' fails', '');
  }

  onCheckLDAPBindConnect(ldap: AuthLdap) {
    this._authService.LdapBindCheck(this.authPolicy.getFormGroupValues()).subscribe(
      response => {
        const respAuthPolicy: AuthAuthenticationPolicy = response.body as AuthAuthenticationPolicy;
        const ldapCheckResponse = this.makeLDAPCheckResponse(LDAPCheckType.BIND, ldap, respAuthPolicy);
        const ldapBindCheckResponseError = AuthPolicyUtil.processLDAPCheckResponse(ldapCheckResponse);
        if (ldapBindCheckResponseError.errors.length > 0) {
          this._ldapBindCheckResponse = ldapCheckResponse;
          this.handleLDAPServerCheckFailure(LDAPCheckType.BIND);
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

  /**
   * This API serves html template
   * @param isNewLDAP
   *
   * This is similar to onInvokeSaveLDAP(..) API
   */
  onInvokeSaveRadius(isNewRadius: boolean) {
    this.saveAuthenticationPolicy();
  }

  saveAuthenticationPolicy() {
    let handler: Observable<{ body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number }>;
    // If the user is in the UI, they must have setup at least a local auth policy. We should only be updating the current one.
    handler = this._authService.UpdateAuthenticationPolicy(this.authPolicy.getFormGroupValues());
    handler.subscribe(
      (response) => {
        this.invokeSuccessToaster('Update Successful', 'Updated Authentication policy. System will log you out in 3 seconds');
        // update data so auth-policy UI will get refresh and UI remain in auth-policy page.
        const body = response.body;
        this.authPolicy = new AuthAuthenticationPolicy(body);
        this.setupToolbarItems();
        // Since we change auth-policy, we have to log-out user.
        const setTime1 = window.setTimeout(() => {
          this._controllerService.publish(Eventtypes.LOGOUT, { 'reason': 'Authentication Policy Update.' });
          window.clearTimeout(setTime1);
        }, 3000);
      },
      this.restErrorHandler('Update Failed')
    );
  }
}
