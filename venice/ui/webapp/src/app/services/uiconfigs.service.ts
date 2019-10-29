import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { AUTH_BODY } from '@app/core';
import { ActivatedRouteSnapshot, Resolve, Router, RouterStateSnapshot } from '@angular/router';
import { Subject, of, forkJoin } from 'rxjs';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { IAuthOperationStatus, IAuthSubjectAccessReviewRequest, AuthPermission_actions, IAuthOperation, IAuthUser, AuthOperation_action } from '@sdk/v1/models/generated/auth';
import { ControllerService } from './controller.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Utility } from '@app/common/Utility';
import { AuthService } from './auth.service';

export enum Features {
  help = 'help',
  workloadWidgets = 'workloadWidgets',
  showDebugMetrics = 'showDebugMetrics',
  troubleshooting = 'troubleshooting'
}

interface UIConfig {
  'enabled-features': Features[];
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
export class UIConfigsService {
  configFile;

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
    'cluster/naples': {
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
    'monitoring/fwlogs': {
      requiredPerm: [
        UIRolePermissions.fwlogsquery_read,
      ],
    },
    'monitoring/fwlogs/fwlogpolicies': {
      requiredPerm: [
        UIRolePermissions.monitoringfwlogpolicy_read,
      ],
    },
    'monitoring/flowexport': {
      requiredPerm: [
        UIRolePermissions.monitoringflowexportpolicy_read,
      ],
    },
    'admin/techsupport': {
      requiredPerm: [
        UIRolePermissions.monitoringtechsupportrequest_read,
      ],
    },
  };

  private _userPermissions: IAuthOperationStatus[] = [];
  private uiPermissions: { [key: string]: boolean } = {};

  get userPermissions() {
    return this._userPermissions;
  }

  set userPermissions(value: IAuthOperationStatus[]) {
    this._userPermissions = value;
    this.setUIPermissions();
  }

  constructor(protected router: Router,
    protected controllerService: ControllerService,
    protected authService: AuthService,
    protected http: HttpClient
  ) {
    this.controllerService.subscribe(Eventtypes.FETCH_USER_PERMISSIONS, (payload) => {
      this.getUserObj().subscribe(
        (resp: any) => {
          if (resp != null) {
            const body = resp.body as IAuthUser;
            this.userPermissions = body.status['access-review'];
          }
        },
        (error) => {
          console.warn('Failed to get user permissions', error);
        }
      );
    });
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
    const enabledFeatures: string[] = this.configFile['enabled-features'];
    return enabledFeatures.some((elem) => {
      return elem.toLowerCase() === featureName.toLowerCase();
    });
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
