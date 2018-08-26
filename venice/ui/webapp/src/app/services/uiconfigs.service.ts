import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { ActivatedRouteSnapshot, Resolve, Router, RouterStateSnapshot } from '@angular/router';
import { Observable } from 'rxjs/Observable';
import { Subject } from 'rxjs/Subject';

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

const CONFIG_FILENAME = "config.json"

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
    'monitoring/alertsevents/alertpolicies': {
      required: [
        'AlertPolicies'
      ],
      default: [
        'AlertPolicies',
        'AlertDestinations'
      ]
    },
    'monitoring/alertsevents/': {
      default: [
        'Alerts',
        'Events'
      ]
    }
  };

  constructor(protected router: Router,
    protected http: HttpClient
  ) { }

  /**
   * We check the route, and if it isn't allowed,
   * we redirect to the homepage and return false to
   * the guard calling this function
   *
   * @param route
   */
  verifySubRoute(route: string) {
    if (!this.canActivateSubRoute(route)) {
      this.navigateToHomepage();
      return false;
    }
    return true;
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
    this.router.navigate(['dashboard']);
  }

  loadConfig() {
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
      return Observable.of();
    }
  }

  isConfigLoaded(): boolean {
    return this.configFile != null;
  }

}


@Injectable()
export class UIConfigsResolver implements Resolve<any> {
  constructor(protected config: UIConfigsService) { }

  resolve(route: ActivatedRouteSnapshot, state: RouterStateSnapshot) {
    return this.config.loadConfig();
  }
}
