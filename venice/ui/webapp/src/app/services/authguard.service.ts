import { Injectable } from '@angular/core';
import { Router, CanActivate, ActivatedRouteSnapshot, RouterStateSnapshot } from '@angular/router';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/auth.service';

@Injectable()
export class AuthGuard implements CanActivate {

  constructor(private router: Router,
    private _controllerService: ControllerService,
    private _authService: AuthService
  ) { }

  canActivate(route: ActivatedRouteSnapshot, state: RouterStateSnapshot) {
    if (this._controllerService.isUserLogin()) {
      return true;
    }

    // not logged in so redirect to login page
    this._authService.redirectUrl = state.url;
    this.router.navigate(['login']);
    return false;
  }
}
