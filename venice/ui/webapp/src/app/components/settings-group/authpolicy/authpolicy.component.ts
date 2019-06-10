import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { FormControl } from '@angular/forms';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { LDAPCheckResponse, LDAPCheckType } from '@app/components/settings-group/authpolicy/.';
import { AuthPolicyUtil } from '@app/components/settings-group/authpolicy/AuthPolicyUtil';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { AuthAuthenticationPolicy, AuthLdap, IApiStatus, IAuthAuthenticationPolicy, AuthAuthenticators_authenticator_order, AuthRadius, IAuthTokenSecretRequest, AuthTokenSecretRequest } from '@sdk/v1/models/generated/auth';
import { Observable } from 'rxjs';
import { Eventtypes } from '@app/enum/eventtypes.enum';


/**
 * AuthpolicyComponent allow user to manage authentication policy.
 * When Venice is setup, there must be day-0 opertion to create auth-policy so that user can login at first place.
 * Thus, the component manage update/test operations.
 *
 * This component internally contain  local, ldap, radius sub components.  It handles REST call on behalf of sub-component. For example, it manages LDAP component's test-server calls.
 *
 * Note: authpolicy.component.html includes two <app-ldap> and <app-radius> tags.  One for 'update' and the other for 'create', developer must be careful to wire up @Input and @Output.
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
  // Allows accessing enum value in html
  authTypes = AuthAuthenticators_authenticator_order;

  secretFormControl: FormControl = new FormControl('', []);
  enableUpdateSecretButton: boolean = false;
  authOrder = ['LOCAL', 'LDAP', 'RADIUS'];
  authPolicy: AuthAuthenticationPolicy = new AuthAuthenticationPolicy({ spec: { authenticators: { 'authenticator-order': ['LOCAL', 'LDAP'] } } });

  // these variables will pass to LDAPComponent
  _ldapConnCheckResponse: LDAPCheckResponse = null;
  _ldapBindCheckResponse: LDAPCheckResponse = null;

  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService,
  ) {
    super(_controllerService);
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
        cssClass: 'global-button-primary authpolicy-toolbar-button authpolicy-toolbar-button-refresh',
        text: 'Refresh',
        callback: () => {
          this.getAuthenticationPolicy();
        }
      },
      {
        cssClass: 'global-button-primary authpolicy-toolbar-button',
        text: 'New Authorization Token',
        callback: () => {
          this.onGenerate();
        }
      }

    ];
    if (rankChanged === true) {
      const saveRankChange = {
        cssClass: 'global-button-primary authpolicy-toolbar-button authpolicy-toolbar-button-save',
        text: 'Save',
        callback: () => {
          this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues());
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
      this._controllerService.restErrorHandler('Failed to get Authentication Policy')
    );
  }

  onGenerate() {
    this._controllerService.invokeConfirm({
      header: 'Please confirm that you want to generate a new authorization token',
      message: 'System will log out all users after the token is generated',
      acceptLabel: 'Yes',
      accept: () => {
        this.generateToken();
      }
    });
  }
  generateToken() {
    const tokenrequest: IAuthTokenSecretRequest = new AuthTokenSecretRequest();
    const processor = this._authService.TokenSecretGenerate(tokenrequest);
    processor.subscribe(
      (response) => {
        this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Generated new token successfully. All users will be logged out in 3 seconds');
        this.setupToolbarItems();
        const setTime1 = window.setTimeout(() => {
          this._controllerService.publish(Eventtypes.LOGOUT, { 'reason': 'Authentication Policy Update.' });
          window.clearTimeout(setTime1);
        }, 3000);
      },
      this._controllerService.restErrorHandler('Error occured: token could not be generated')
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

  getLDAPTestError(ldapCheckType: LDAPCheckType): any {
    const error = {
      statusCode: 200,
      body: {
        message: (ldapCheckType === LDAPCheckType.CONNECTION ) ? 'LDAP Connection Test Failed' : 'LDAP Bind Test Failed.'
      }
    };
    return error;
  }

  onCheckLDAPServerConnect(ldap: AuthLdap) {
    this.authPolicy.spec.authenticators.ldap = ldap;
    this._authService.LdapConnectionCheck(this.authPolicy.getFormGroupValues()).subscribe(
      response => {
        const respAuthPolicy: AuthAuthenticationPolicy = response.body as AuthAuthenticationPolicy;
        const ldapCheckResponse = this.makeLDAPCheckResponse(LDAPCheckType.CONNECTION, ldap, respAuthPolicy);
        const connCheckResponseError = AuthPolicyUtil.processLDAPCheckResponse(ldapCheckResponse);
        if (connCheckResponseError.errors.length > 0) {
          this._ldapConnCheckResponse = ldapCheckResponse;
          this.handleLDAPServerCheckFailure(LDAPCheckType.CONNECTION, this.getLDAPTestError(LDAPCheckType.CONNECTION));
        } else {
          this.handleLDAPServerCheckSuccess(LDAPCheckType.CONNECTION);
        }
      },
      this._controllerService.restErrorHandler('Failed to check LDAP server configuration')
    );
  }

  handleLDAPServerCheckSuccess(type: LDAPCheckType) {
    this._controllerService.invokeSuccessToaster('Test LDAP ' + type + ' passed', '');
  }

  handleLDAPServerCheckFailure(type: LDAPCheckType, error: any) {
    this._controllerService.invokeRESTErrorToaster('Test LDAP ' + type + ' failed', error);
  }

  onCheckLDAPBindConnect(ldap: AuthLdap) {
    this.authPolicy.spec.authenticators.ldap = ldap;
    this._authService.LdapBindCheck(this.authPolicy.getFormGroupValues()).subscribe(
      response => {
        const respAuthPolicy: AuthAuthenticationPolicy = response.body as AuthAuthenticationPolicy;
        const ldapCheckResponse = this.makeLDAPCheckResponse(LDAPCheckType.BIND, ldap, respAuthPolicy);
        const ldapBindCheckResponseError = AuthPolicyUtil.processLDAPCheckResponse(ldapCheckResponse);
        if (ldapBindCheckResponseError.errors.length > 0) {
          this._ldapBindCheckResponse = ldapCheckResponse;
          this.handleLDAPServerCheckFailure(LDAPCheckType.BIND, this.getLDAPTestError(LDAPCheckType.BIND));
        } else {
          this.handleLDAPServerCheckSuccess(LDAPCheckType.BIND);
        }
      },
      this._controllerService.restErrorHandler('Failed to check LDAP server configuration')
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
    this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues());
  }

  /**
   * This API serves html template
   * @param isNewLDAP
   *
   * This is similar to onInvokeSaveLDAP(..) API
   */
  onInvokeSaveRadius(isNewRadius: boolean) {
    this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues());
  }

  saveAuthenticationPolicy(authAuthenticationPolicy: IAuthAuthenticationPolicy) {
    let handler: Observable<{ body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number }>;
    // If the user is in the UI, they must have setup at least a local auth policy. We should only be updating the current one.
    handler = this._authService.UpdateAuthenticationPolicy(authAuthenticationPolicy);
    handler.subscribe(
      (response) => {
        this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Updated Authentication policy. System will log you out in 3 seconds');
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
      this._controllerService.restErrorHandler(Utility.UPDATE_FAILED_SUMMARY)
    );
  }

  onInvokeCreateLDAP(ldap: AuthLdap) {
    this.authPolicy.spec.authenticators['authenticator-order'].push(AuthAuthenticators_authenticator_order.LDAP);
    this.authPolicy.spec.authenticators.ldap = ldap;
    this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues());
  }

  onInvokeCreateRadius(radius: AuthRadius) {
    this.authPolicy.spec.authenticators['authenticator-order'].push(AuthAuthenticators_authenticator_order.RADIUS);
    this.authPolicy.spec.authenticators.radius = radius;
    this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues());
  }

  onInvokeRemoveRadius(radius: AuthRadius) {
    this._onInvokeRemoveConfigHelper(AuthAuthenticators_authenticator_order.RADIUS);
  }

  onInvokeRemoveLDAP(ldap: AuthLdap) {
    this._onInvokeRemoveConfigHelper(AuthAuthenticators_authenticator_order.LDAP);
  }

  _onInvokeRemoveConfigHelper(type: any) {
    const saveData = this.authPolicy.getFormGroupValues(); // extract data out first
    const configIndex = saveData.spec.authenticators['authenticator-order'].findIndex((value) => value === type);
    saveData.spec.authenticators['authenticator-order'].splice(configIndex, 1);
    delete saveData.spec.authenticators[type.toLowerCase()]; // remove radius or ldap
    this.saveAuthenticationPolicy(saveData);
  }
}
