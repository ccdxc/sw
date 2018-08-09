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
      if (pathSnapshot.url.length > 0) {
        path.push(pathSnapshot.url[0].path);
      }
    });
    return this.uiconfigsService.verifySubRoute(path.join('/'));

  }
}
