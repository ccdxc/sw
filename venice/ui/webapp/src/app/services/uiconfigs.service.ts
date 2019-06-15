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

interface UIConfig {
  'disabled-objects': string[];
  'disabled-features': string[];
}

interface PageRequirement {
  required?: string[];
  default?: string[];
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
  pageRequirements: PageRequirementMap = {
    'cluster/cluster': {
      required: [
        UIRolePermissions.clustercluster_read,
      ],
    },
    'cluster/:id': {
      required: [
        UIRolePermissions.clusternode_read,
      ],
    },
    'cluster/naples': {
      required: [
        UIRolePermissions.clustersmartnic_read,
      ],
    },
    'cluster/hosts': {
      required: [
        UIRolePermissions.clusterhost_read,
      ],
    },
    '/workload': {
      required: [
        UIRolePermissions.workloadworkload_read,
      ],
    },
    '/security/sgpolicies': {
      required: [
        UIRolePermissions.securitysgpolicy_read,
      ],
    },
    '/security/securityapps': {
      required: [
        UIRolePermissions.securityapp_read,
      ],
    },
    'monitoring/alertsevents/': {
      default: [
        UIRolePermissions.monitoringalert_read,
        UIRolePermissions.eventsevent_read,
      ]
    },
    'monitoring/alertsevents/alertpolicies': {
      required: [
        UIRolePermissions.monitoringalertpolicy_read,
      ],
    },
    'monitoring/alertsevents/eventpolicy': {
      default: [
        UIRolePermissions.monitoringeventpolicy_read,
      ]
    },
    '/monitoring/fwlogs': {
      required: [
        UIRolePermissions.fwlogsquery_read,
      ],
    },
    '/monitoring/fwlogs/fwlogpolicies': {
      required: [
        UIRolePermissions.monitoringfwlogpolicy_read,
      ],
    },
    '/monitoring/flowexport': {
      required: [
        UIRolePermissions.monitoringflowexportpolicy_read,
      ],
    },
    '/monitoring/techsupport': {
      required: [
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
    this._userPermissions.forEach( (permission) => {
      if (permission.allowed) {
        let cat = permission.operation.resource.group;
        if (cat == null) {
          cat = '';
        }
        const kind = permission.operation.resource.kind;
        const action = permission.operation.action;
        if (action === AuthOperation_action.AllActions) {
          Object.keys(AuthPermission_actions).forEach( (a) => {
            const key = _.toLower(cat) + _.toLower(kind) + '_' + _.toLower(a);
            this.uiPermissions[key] = permission.allowed;
          });
        } else {
          const key = _.toLower(cat) + _.toLower(kind) + '_' + _.toLower(action);
          this.uiPermissions[key] = permission.allowed;
        }
      }
    });

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
    if (this.pageRequirements[route] == null) {
      return true;
    }
    const req = this.pageRequirements[route].required;
    if (req != null && req.length !== 0) {
      // If one of the required objects are disabled, we return false
      const reqResult = req.some((obj) => {
        return this.isObjectDisabled(obj);
      });
      if (reqResult) {
        return false;
      }
    }

    const defaultObjects = this.pageRequirements[route].default;
    if (defaultObjects != null && defaultObjects.length !== 0) {
      // If one of the objects isn't disabled, we allow the route
      const defaultObjRes = defaultObjects.some((obj) => {
        return !this.isObjectDisabled(obj);
      });
      return defaultObjRes;
    }
    return true;
  }

  /**
   * If we don't have a config file, we by default
   * allow all objects
   * @param objName
   */
  isObjectDisabled(objName: string): boolean {
    if (this.configFile == null) {
      return false;
    }
    const disabledObjs: string[] = this.configFile['disabled-objects'];
    return disabledObjs.some((elem) => {
      return elem.toLowerCase() === objName.toLowerCase();
    });
  }

  /**
   * If we don't have a config file, we by default
   * allow all features
   * @param featureName
   */
  isFeatureDisabled(featureName: string): boolean {
    if (this.configFile == null) {
      return false;
    }
    const disabledFeatures: string[] = this.configFile['disabled-features'];
    return disabledFeatures.some((elem) => {
      return elem.toLowerCase() === featureName.toLowerCase();
    });
  }

  navigateToHomepage() {
    // Changing homepage for August release
    // this.router.navigate(['dashboard']);
    this.router.navigate(['cluster', 'cluster']);
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
      return !req.some( (p) => {
        if (!this.isAuthorized(p)) {
          return true;
        }
      });
    }

    if (opt != null && opt.length !== 0) {
      // If all of the optional objects are disabled, we return false
      return opt.some( (p) => {
        return this.isAuthorized(p);
      });
    }

    return true;
  }

  isAuthorized(permission: UIRolePermissions) {
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
