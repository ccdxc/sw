import { Injectable } from '@angular/core';
import { ActivatedRouteSnapshot, CanActivate, CanActivateChild, RouterStateSnapshot } from '@angular/router';
import { UIConfigsService } from '@app/services/uiconfigs.service';

@Injectable()
export class RouteGuard implements CanActivate, CanActivateChild {

  constructor(private uiconfigsService: UIConfigsService
  ) { }

  canActivate(route: ActivatedRouteSnapshot, state: RouterStateSnapshot) {
    return this.verifyRoute(route);
  }

  canActivateChild(route: ActivatedRouteSnapshot, state: RouterStateSnapshot) {
    return this.verifyRoute(route);
  }

  verifyRoute(route: ActivatedRouteSnapshot): boolean {
    const path = [];
    route.pathFromRoot.forEach((pathSnapshot) => {
      pathSnapshot.url.forEach((url) => {
        path.push(url.path);
      });
    });
    if (!this.uiconfigsService.canActivateSubRoute(path.join('/'))) {
      this.uiconfigsService.navigateToHomepage();
      return false;
    }
    return true;

  }
}
