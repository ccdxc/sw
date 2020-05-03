import { HttpClient } from '@angular/common/http';
import { Injectable, OnDestroy } from '@angular/core';
import { AUTH_BODY } from '@app/core';
import { ActivatedRouteSnapshot, Resolve, Router, RouterStateSnapshot } from '@angular/router';
import { Subject, of, forkJoin, Subscription } from 'rxjs';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { IAuthOperationStatus, IAuthSubjectAccessReviewRequest, AuthPermission_actions, IAuthOperation, IAuthUser, AuthOperation_action, AuthOperation } from '@sdk/v1/models/generated/auth';
import { ControllerService } from './controller.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Utility } from '@app/common/Utility';
import { AuthService } from './auth.service';
import { LicenseService } from './license.service';
import { IClusterLicense } from '@sdk/v1/models/generated/cluster';

type derivedFeaturesFn = (configJson: UIConfig, licenseObj: IClusterLicense) => boolean;

export enum UIFeatures {
  // Config json features
  fwlogs = 'fwlogs',
  workloadWidgets = 'workloadWidgets',
  showDebugMetrics = 'showDebugMetrics',
  troubleshooting = 'troubleshooting',
  securitygroup = 'securitygroup',
  dataCache = 'dataCache',
  apiCapture = 'apiCapture',

  // derived features
  cloud = 'cloud',
  enterprise = 'enterprise',
}

// Any features that are computed based on multipe attributes
// from config.json or license object
const derivedFeatures: { [key in UIFeatures]?: derivedFeaturesFn } = {
  cloud: (configJson, licenseObj) => {
    return licenseObj != null;
  },
  enterprise: (configJson, licenseObj) => {
    return licenseObj == null;
  },
};


// TODO: this should come from venice-sdk once PSM feature flags are moved to proto file
export enum BackendFeatures {
  OverlayRouting         = 'OverlayRouting',
  SubnetSecurityPolicies = 'SubnetSecurityPolicies',
  SecurityALG            = 'SecurityALG',
}

export const Features = { ...UIFeatures, ...BackendFeatures };
export type Features =  UIFeatures | BackendFeatures;

interface MetricsQuery {
  startTimeLength?: string;
}

export interface UIConfig {
  'enabled-features': Features[];
  'disabled-features': Features[]; // Used for force disbaling backend features
  'metricsQuery'?: MetricsQuery;
}

interface PageRequirement {
  requiredPerm?: UIRolePermissions[];
  defaultPerm?: UIRolePermissions[];
  requiredFeatures?: Features[];
  defaultFeatures?: Features[];
}

interface PageRequirementMap {
  [url: string]: PageRequirement;
}

const CONFIG_FILENAME = 'config.json';

@Injectable()
export class UIConfigsService implements OnDestroy {
  // For each url listed, we list object dependenies
  // For the required field, if any of the objects are disabled,
  // we disable the route.
  // Otherwise we check default, and if all of the listed objects are
  // disabled we then disable the route
  // Else, it is enabled
  // NOTE: If a subroute is not allowed, further nested paths are automatically blocked
  // Ex. if monitoring is blocked, monitoring/alertsevevnts will be blocked without checking
  // ROUTES SHOULD NOT START WITH A SLASH
  pageRequirements: PageRequirementMap = {
    'cluster/cluster': {
      requiredPerm: [
        UIRolePermissions.clustercluster_read,
      ],
    },
    'cluster/:id': {
      requiredPerm: [
        UIRolePermissions.clusternode_read,
      ],
    },
    'cluster/dscs': {
      requiredPerm: [
        UIRolePermissions.clusterdistributedservicecard_read,
      ],
    },
    'cluster/hosts': {
      requiredPerm: [
        UIRolePermissions.clusterhost_read,
      ],
    },
    'workload': {
      requiredPerm: [
        UIRolePermissions.workloadworkload_read,
      ],
    },
    'security/sgpolicies': {
      requiredPerm: [
        UIRolePermissions.securitynetworksecuritypolicy_read,
      ],
    },
    'security/securityapps': {
      requiredPerm: [
        UIRolePermissions.securityapp_read,
      ],
    },
    'security/securitygroups': {
      requiredFeatures: [
        Features.securitygroup,
      ],
      requiredPerm: [
        UIRolePermissions.securitysecuritygroup_read,
      ]
    },
    'monitoring/alertsevents/': {
      defaultPerm: [
        UIRolePermissions.monitoringalert_read,
        UIRolePermissions.eventsevent_read,
      ]
    },
    'monitoring/troubleshooting/': {
      requiredFeatures: [
        Features.troubleshooting,
      ]
    },
    'monitoring/alertsevents/alertpolicies': {
      requiredPerm: [
        UIRolePermissions.monitoringalertpolicy_read,
      ],
    },
    'monitoring/alertsevents/eventpolicy': {
      defaultPerm: [
        UIRolePermissions.monitoringeventpolicy_read,
      ]
    },
    'monitoring/archive': {
      requiredPerm: [
        UIRolePermissions.monitoringarchiverequest_read,
        UIRolePermissions.eventsevent_read,
        UIRolePermissions.auditevent_read,
        UIRolePermissions.objstoreobject_read
      ],
    },
    'monitoring/fwlogs': {
      requiredFeatures: [
        Features.fwlogs,
      ],
      requiredPerm: [
       UIRolePermissions.fwlog_read,
      ],
    },
    'monitoring/flowexport': {
      requiredPerm: [
        UIRolePermissions.monitoringflowexportpolicy_read,
      ],
    },
    'troubleshoot/troubleshooting': {
      requiredPerm: [
        UIRolePermissions.adminrole,
      ],
      requiredFeatures: [
        Features.troubleshooting,
      ]
    },
    'admin/techsupport': {
      requiredPerm: [
        UIRolePermissions.monitoringtechsupportrequest_read,
      ],
    },
    'admin/api': {
      requiredPerm: [
        UIRolePermissions.adminrole,
      ],
      requiredFeatures: [
        Features.apiCapture,
      ]
    },
  };

  private _userPermissions: IAuthOperationStatus[] = [];
  private _licenseObj: IClusterLicense;
  private _configFile: UIConfig;
  private features: { [key: string]: boolean } = {};
  private uiPermissions: { [key: string]: boolean } = {};


  get userPermissions() {
    return this._userPermissions;
  }

  set userPermissions(value: IAuthOperationStatus[]) {
    this._userPermissions = value;
    this.setUIPermissions();
  }

  get licenseObj() {
    return this._licenseObj;
  }

  set licenseObj(value: IClusterLicense) {
    this._licenseObj = value;
    this.setFeatures();
  }

  get configFile() {
    return this._configFile;
  }

  set configFile(value: UIConfig) {
    this._configFile = value;
    this.setFeatures();
  }

  subscriptions: Subscription[] = [];

  constructor(protected router: Router,
    protected controllerService: ControllerService,
    protected authService: AuthService,
    protected http: HttpClient,
    protected licenseService: LicenseService,
  ) {
    let sub = this.controllerService.subscribe(Eventtypes.FETCH_USER_PERMISSIONS, (payload) => {
      this.getUserObj().subscribe(
        (resp: any) => {
          if (resp != null) {
            const body = resp.body as IAuthUser;
            this.userPermissions = body.status['access-review'];
            this.checkMetricPermissions();
          }
        },
        (error) => {
          console.warn('Failed to get user permissions', error);
        }
      );
    });
    this.subscriptions.push(sub);
    // Attempt on load, and refetch if user signs out/in
    // Adding timeout to push this call to the back of the event queue.
    // prevents http type error due to services not being initialized yet.
    setTimeout(() => {
      const sub1 = this.licenseService.GetLicense().subscribe(
        (response) => {
          // Setting the property will trigger updates to features
          this.licenseObj = response.body as IClusterLicense;
        },
        (error) => {
          this.licenseObj = null;
        }
      );
      this.subscriptions.push(sub1);
    }, 0);
    sub = this.controllerService.subscribe(Eventtypes.LOGIN_SUCCESS, (payload) => {
      // TODO: switch to watch once it's added
      const sub2 = this.licenseService.GetLicense().subscribe(
        (response) => {
          // Setting the property will trigger updates to features
          this.licenseObj = response.body as IClusterLicense;
        },
        (error) => {
          this.licenseObj = null;
        }
      );
      this.subscriptions.push(sub2);
    });
    this.subscriptions.push(sub);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(s => {
      s.unsubscribe();
    });
  }

  setFeatures() {
    // Populate features
    const _ = Utility.getLodash();
    this.features = {};

    // First check backend flags
    if (this._licenseObj != null && this._licenseObj.spec != null && this._licenseObj.spec.features != null) {
      // TODO: Use status instead of spec once it is populated
      this._licenseObj.spec.features.forEach(elem => {
        if (Features[elem['feature-key']] == null) {
          console.error('Unrecognized backend feature flag ' + elem['feature-key']);
        }
        this.features[elem['feature-key']] = true;
      });
    }

    // call any registered callbacks for derived features
    Object.keys(derivedFeatures).forEach(key => {
      this.features[key] = derivedFeatures[key](this._configFile, this._licenseObj);
    });

    // Then load config json flags
    // this allows config file to supercede all
    if (this._configFile != null) {
      const enabledFeatures: string[] = this._configFile['enabled-features'];
      if (enabledFeatures != null) {
        enabledFeatures.forEach(elem => {
          if (Features[elem] == null) {
            console.error('Unrecognized feature flag ' + elem);
          }
          this.features[elem] = true;
        });
      }
      const disabledFeatures: string[] = this._configFile['disabled-feature'];
      if (disabledFeatures != null) {
        disabledFeatures.forEach(elem => {
          if (Features[elem] == null) {
            console.error('Unrecognized feature flag ' + elem);
          }
          if (this.features[elem]) {
            this.features[elem] = false;
          }
        });
      }
    }

    // Publish to roleGuards that features have been changed
    this.controllerService.publish(Eventtypes.NEW_FEATURE_PERMISSIONS, null);
  }

  setUIPermissions() {
    const _ = Utility.getLodash();
    this.uiPermissions = {};
    this._userPermissions.forEach((permission) => {
      if (permission.allowed) {
        let cat = permission.operation.resource.group;
        if (cat == null) {
          cat = '';
        }
        const kind = permission.operation.resource.kind;
        const action = permission.operation.action;
        if (action === AuthOperation_action['all-actions']) {
          Object.keys(AuthPermission_actions).forEach((a) => {
            const key = _.toLower(cat) + _.toLower(kind) + '_' + _.toLower(a);
            this.uiPermissions[key] = permission.allowed;
          });
        } else {
          const key = _.toLower(cat) + _.toLower(kind) + '_' + _.toLower(action);
          this.uiPermissions[key] = permission.allowed;
        }
      }
    });
    if (Utility.getInstance().isAdmin()) {
      this.uiPermissions[UIRolePermissions.adminrole] = true;
    }

    // Publish to roleGuards that UI permissions have been changed
    this.controllerService.publish(Eventtypes.NEW_USER_PERMISSIONS, null);

    // Request metric permissions
  }

  checkMetricPermissions() {
    const authBody = JSON.parse(sessionStorage.getItem(AUTH_BODY));
    if (authBody != null && authBody.meta != null && authBody.meta.name != null) {
      const req: IAuthSubjectAccessReviewRequest = {
        meta: {
          name: authBody.meta.name,
          tenant: authBody.meta.tenant,
        },
        operations: [{
          resource: {
            tenant: authBody.meta.tenant,
            // This kind does not need to be an actual
            // table name
            // We are just checking that we can read arbitrary tables from the db
            kind: 'randomKind',
          },
          action: AuthOperation_action.read
        }],
      };
      this.authService.IsAuthorized(authBody.meta.name, req).subscribe(
        (resp: any) => {
          if (resp != null) {
            const body = resp.body as IAuthUser;
            const hasMetricPerm = body.status['access-review'].some(v => {
              return v.allowed;
            });
            if (hasMetricPerm) {
              const perm: IAuthOperationStatus = {
                allowed: true,
                operation: {
                  resource: {
                    kind: 'metrics',
                  },
                  action: AuthOperation_action.read,
                }
              };
              body.status['access-review'].push(perm);
            }
            const userPermissions = this.userPermissions;
            this.userPermissions = userPermissions.concat(body.status['access-review']);
          }
        },
        (error) => {
          console.warn('Failed to get user permissions for metrics', error);
        }
      );
    }
  }



  /**
   * Looks up the route in the pageRequirements object
   * If the url isn't present it is allowed by default
   * Otherwise we follow the algorithm stated above pageRequirements
   * @param route
   */
  canActivateSubRoute(route: string): boolean {

    if (route === 'maintenance') {
      return Utility.getInstance().getMaintenanceMode();
    }

    if (this.pageRequirements[route] == null) {
      return true;
    }
    // permission guarding
    const req = this.pageRequirements[route].requiredPerm;
    if (req != null && req.length !== 0) {
      // If one of the required objects are disabled, we return false
      const reqResult = req.some((obj) => {
        return !this.isAuthorized(obj);
      });
      if (reqResult) {
        return false;
      }
    }

    const defaultObjects = this.pageRequirements[route].defaultPerm;
    if (defaultObjects != null && defaultObjects.length !== 0) {
      // If one of the objects isn't disabled, we allow the route
      const defaultObjRes = defaultObjects.some((obj) => {
        return this.isAuthorized(obj);
      });
      if (!defaultObjRes) {
        return false;
      }
    }

    // Checking feature guarding
    const reqFeatures = this.pageRequirements[route].requiredFeatures;
    if (reqFeatures != null && reqFeatures.length !== 0) {
      // If one of the required objects are disabled, we return false
      const reqResult = reqFeatures.some((obj) => {
        return !this.isFeatureEnabled(obj);
      });
      if (reqResult) {
        return false;
      }
    }

    const defaultFeatures = this.pageRequirements[route].defaultFeatures;
    if (defaultFeatures != null && defaultFeatures.length !== 0) {
      // If one of the objects isn't disabled, we allow the route
      const defaultRes = defaultFeatures.some((obj) => {
        return this.isFeatureEnabled(obj);
      });
      if (!defaultRes) {
        return false;
      }
    }
    return true;
  }

  /**
   * If we don't have a config file, we by default
   * allow all features
   * @param featureName
   */
  isFeatureEnabled(featureName: string): boolean {
    if (this.configFile == null) {
      return true;
    }
    if (this.features[featureName] == null) {
      return false;
    }
    return this.features[featureName];
  }

  featureGuardIsEnabled(req: Features[], opt: Features[]): boolean {
    if (req != null && req.length !== 0) {
      // If one of the required objects are disabled, we return false
      return !req.some((p) => {
        if (!this.isFeatureEnabled(p)) {
          return true;
        }
      });
    }

    if (opt != null && opt.length !== 0) {
      // If all of the optional objects are disabled, we return false
      return opt.some((p) => {
        return this.isFeatureEnabled(p);
      });
    }

    return true;
  }

  navigateToHomepage() {
    this.router.navigate(['dashboard']);
  }

  loadConfig() {
    // Setting UI permissions from local storage object
    this.setUserPermissionsFromLocalStorage();

    // Only want to do this once - if root page is revisited, it calls this again.
    if (this.configFile == null) {
      let baseUrl = window.location.protocol + '//' + window.location.hostname + ':' + window.location.port;
      baseUrl += '/assets/' + CONFIG_FILENAME;
      const observable = new Subject();
      this.http.get(baseUrl).subscribe(
        (configData: UIConfig) => {
          // set config data
          this.configFile = configData;
          if (this.configFile == null) {
            console.log('UI Configuration not specified');
          }
          // Resolve the promise
          observable.next();
          observable.complete();
        },
        (error) => {
          console.warn('Failed to get user configurations', error);
          // Resolve the promise as normal
          // If this file is missing we load the UI as if the
          // user had everything enabled
          observable.next();
          observable.complete();
        }
      );
      return observable;
    } else {
      return of();
    }
  }

  setUserPermissionsFromLocalStorage() {
    const authBody = JSON.parse(sessionStorage.getItem(AUTH_BODY));
    if (authBody != null && authBody.status != null && authBody.status['access-review'] != null) {
      this.userPermissions = authBody.status['access-review'];
      this.checkMetricPermissions();
    } else {
      // remove all permissions
      this.userPermissions = [];
    }
  }

  getUserObj() {
    const authBody = JSON.parse(sessionStorage.getItem(AUTH_BODY));
    if (authBody != null && authBody.meta != null && authBody.meta.name != null) {
      return this.authService.GetUser(authBody.meta.name);
    } else {
      // This should never happen. If username is null we shouldn't have been logged in
      console.error('User was none when attempting to get role permissions');
      return of();
    }
  }

  isConfigLoaded(): boolean {
    return this.configFile != null;
  }

  roleGuardIsAuthorized(req: UIRolePermissions[], opt: UIRolePermissions[]): boolean {
    if (req != null && req.length !== 0) {
      // If one of the required objects are disabled, we return false
      return !req.some((p) => {
        if (!this.isAuthorized(p)) {
          return true;
        }
      });
    }

    if (opt != null && opt.length !== 0) {
      // If all of the optional objects are disabled, we return false
      return opt.some((p) => {
        return this.isAuthorized(p);
      });
    }

    return true;
  }

  isAuthorized(permission: UIRolePermissions) {
    // VS-802 start. User may refresh a page, we want browser to stay in current page. Thus, we restore permission objects from session-storage
    if (this.controllerService.isUserLogin()) {
      if (this.userPermissions.length <= 0) {
        this.setUserPermissionsFromLocalStorage();
      }
      if (Utility.isEmptyObject(this.uiPermissions)) {
        this.setUIPermissions();
      }
    }
    // VS-802 end.
    return !!this.uiPermissions[permission];
  }
}


@Injectable()
export class UIConfigsResolver implements Resolve<any> {
  constructor(protected config: UIConfigsService) { }

  resolve(route: ActivatedRouteSnapshot, state: RouterStateSnapshot) {
    return this.config.loadConfig();
  }
}
