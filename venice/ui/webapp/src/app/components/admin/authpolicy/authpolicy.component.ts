import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { FormControl } from '@angular/forms';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { LDAPCheckResponse, LDAPCheckType, LdapSave, RadiusSave } from '@app/components/admin/authpolicy/.';
import { AuthPolicyUtil } from '@app/components/admin/authpolicy/AuthPolicyUtil';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { AuthAuthenticationPolicy, AuthLdap, IApiStatus, IAuthAuthenticationPolicy, AuthAuthenticators_authenticator_order, AuthRadius, IAuthTokenSecretRequest, AuthTokenSecretRequest } from '@sdk/v1/models/generated/auth';
import { Observable } from 'rxjs';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import {Icon} from '@app/models/frontend/shared/icon.interface';


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

  bodyicon: Icon = {
    margin: {
      top: '9px',
      left: '8px'
    },
    svgIcon: 'auth-pol'
  };

  // Allows accessing enum value in html
  authTypes = AuthAuthenticators_authenticator_order;

  secretFormControl: FormControl = new FormControl('', []);
  enableUpdateSecretButton: boolean = false;
  authOrder = ['LOCAL', 'LDAP', 'RADIUS'];
  authPolicy: AuthAuthenticationPolicy = new AuthAuthenticationPolicy({ spec: { authenticators: { 'authenticator-order': ['LOCAL', 'LDAP'] } } });

  // these variables are for knowing if we are still in Create Mode or not
  inRadiusCreateMode: boolean = false;
  inLDAPCreateMode: boolean = false;

  // these variables are for knowing if policy has error or not during saving
  ldapHasError: boolean = false;
  radiusHasError: boolean = false;

  // these variables will pass to LDAPComponent
  _ldapConnCheckResponse: LDAPCheckResponse = null;
  _ldapBindCheckResponse: LDAPCheckResponse = null;

  updateDisabled: boolean;

  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService,
    protected uiconfigsService: UIConfigsService
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this.updateDisabled = !this.uiconfigsService.isAuthorized(UIRolePermissions.authauthenticationpolicy_update);
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
          this.setupToolbarItems();
        }
      }
    ];
    if (this.updateDisabled === false) {
      const newAuthToken = {
        cssClass: 'global-button-primary authpolicy-toolbar-button',
        text: 'New Authorization Token',
        callback: () => {
          this.onGenerate();
        }
      };
      buttons.push(newAuthToken);
    }
    if (rankChanged === true) {
      const saveRankChange = {
        cssClass: 'global-button-primary authpolicy-toolbar-button authpolicy-toolbar-button-save',
        text: 'Save',
        callback: () => {
          this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues());
          this.setupToolbarItems();
        }
      };
      buttons.push(saveRankChange);
    }

    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Auth Policy', url: '' }]
    });
  }

  /**
   * Responsible for getting current authentication policy object
   */
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

  /**
   * This function handles changing order of policies
   * @param newRank
   * @param oldRank
   */
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

  /**
   * This function calls LdapConnectionCheck() API.
   * Later it takes appropriate actions as per the response of that API call.
   * @param ldap
   */
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

  /**
   * This function calls LdapBindCheck() API.
   * Later it takes appropriate actions as per the response of that API call.
   * @param ldap
   */
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
   * @param ldap
   *
   * Even through we passes in "ldap" parameter, (ldap is for creating auth-policy), it is not likely ldap will be true.
   * If the user is in the UI, they must have setup at least a local auth policy. We should only be updating the current one.
   */
  onInvokeSaveLDAP(ldap: LdapSave) {
    this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues(), ldap.onSuccess, AuthAuthenticators_authenticator_order.ldap);
  }

  /**
   * This API serves html template
   * @param radius
   *
   * This is similar to onInvokeSaveLDAP(..) API
   */
  onInvokeSaveRadius(radius: RadiusSave) {
    this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues(), radius.onSuccess, AuthAuthenticators_authenticator_order.radius);
  }

  /**
   * saveAuthenticationPolicy() calls the UpdateAuthenticationPolicy API for saving the policies
   * Once the response is received, appropriate actions are performed for the type of response
   *
   * @param authAuthenticationPolicy This parameter stores the authenticators data that needs to be saved
   * @param onSaveSuccess This is a function which is called on save success
   * @param authTypePolicy This parameter tells which policy is getting updated
   */
  saveAuthenticationPolicy(authAuthenticationPolicy: IAuthAuthenticationPolicy, onSaveSuccess: Function | null = null, authTypePolicy: string = null) {
    let handler: Observable<{ body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number }>;
    // If the user is in the UI, they must have setup at least a local auth policy. We should only be updating the current one.

    handler = this._authService.UpdateAuthenticationPolicy(authAuthenticationPolicy, null, null, false, true); // VS-1152 don't trim local:{}
    handler.subscribe(
      (response) => {
        this.authPolicy = new AuthAuthenticationPolicy(authAuthenticationPolicy);
        this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Updated Authentication policy.');
        if (onSaveSuccess != null) {
          onSaveSuccess();
        }
        switch (authTypePolicy) {
          case AuthAuthenticators_authenticator_order.ldap: { this.inLDAPCreateMode = false;
            this.ldapHasError = false;
                                                            break; }
          case AuthAuthenticators_authenticator_order.radius: {this.inRadiusCreateMode = false;
            this.radiusHasError = false;
                                                    break; }
        }
        // per back-end, changing auth-policy does not require user to re-login
      },
      (error) => {
        this._controllerService.invokeRESTErrorToaster(Utility.UPDATE_FAILED_SUMMARY, error);
        switch (authTypePolicy) {
          case AuthAuthenticators_authenticator_order.ldap: { this.ldapHasError = true;
                                                            break; }
          case AuthAuthenticators_authenticator_order.radius: {this.radiusHasError = true;
                                                    break; }
        }
      }
    );
  }

  /**
   * This function invokes creation of LDAP Policy
   * @param ldap This parameter contains the createData and onSuccess fields
   * createData stores the LDAP policy data
   * onSuccess is a function which is executed if saving is successful without any errors
   */
  onInvokeCreateLDAP(ldap: LdapSave) {
    if (!this.ldapHasError) {
      this.authPolicy.spec.authenticators['authenticator-order'].push(AuthAuthenticators_authenticator_order.ldap);
      this.inLDAPCreateMode = true;
    } // During addition of faulty policy, this helps in avoiding pushing in authenticator-order twice or more
    this.authPolicy.spec.authenticators.ldap = ldap.createData;
    this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues(), ldap.onSuccess, AuthAuthenticators_authenticator_order.ldap);
  }

  /**
   * This function invokes creation of Radius Policy
   * @param radius This parameter contains the createData and onSuccess fields
   * createData stores the Radius policy data
   * onSuccess is a function which is executed if saving is successful without any errors
   */
  onInvokeCreateRadius(radius: RadiusSave) {
    if (!this.radiusHasError) {
      this.authPolicy.spec.authenticators['authenticator-order'].push(AuthAuthenticators_authenticator_order.radius);
      this.inRadiusCreateMode = true;
    } // During addition of faulty policy, this helps in avoiding pushing in authenticator-order twice or more
    this.authPolicy.spec.authenticators.radius = radius.createData;
    this.saveAuthenticationPolicy(this.authPolicy.getFormGroupValues(), radius.onSuccess, AuthAuthenticators_authenticator_order.radius);
  }

  /**
   * Deletes radius policy
   * @param radius
   */
  onInvokeRemoveRadius(radius: AuthRadius) {
    this._onInvokeRemoveConfigHelper(AuthAuthenticators_authenticator_order.radius);
    this.inRadiusCreateMode = true;
  }

  /**
   * Deletes ldap policy
   * @param ldap
   */
  onInvokeRemoveLDAP(ldap: AuthLdap) {
    this._onInvokeRemoveConfigHelper(AuthAuthenticators_authenticator_order.ldap);
    this.inLDAPCreateMode = true;
  }

  _onInvokeRemoveConfigHelper(type: any) {
    const saveData = this.authPolicy.getFormGroupValues(); // extract data out first
    const configIndex = saveData.spec.authenticators['authenticator-order'].findIndex((value) => value === type);
    saveData.spec.authenticators['authenticator-order'].splice(configIndex, 1);
    delete saveData.spec.authenticators[type.toLowerCase()]; // remove radius or ldap
    this.saveAuthenticationPolicy(saveData);
  }
}
