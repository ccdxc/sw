import { TestBed,  inject } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { RouterTestingModule } from '@angular/router/testing';
import { Router, RouterStateSnapshot } from '@angular/router';
import { AuthGuard } from '@app/services/authguard.service';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { MatIconRegistry } from '@angular/material';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { AuthService } from '@app/services/auth.service';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { MessageService } from '@app/services/message.service';

class MockControllerService extends ControllerService {
  static isLoggedIn = false;

  isUserLogin(): boolean {
    return MockControllerService.isLoggedIn;
  }
}

describe('AuthGuardService', () => {

  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [AuthGuard,
        { provide: ControllerService, useClass: MockControllerService },
        ConfirmationService,
        MatIconRegistry,
        LogService,
        LogPublishersService,
        AuthService,
        MockControllerService,
        MessageService
      ],
      imports: [
        RouterTestingModule,
        HttpClientTestingModule
      ]
    });

  });

  it('check route guarding',

    // inject your guard service AND Router
    inject([
      AuthGuard,
      Router,
      AuthService
    ],
      (
        authGuard: AuthGuard,
        router,
        authService
      ) => {

        // add a spy
        spyOn(router, 'navigate');
        const state: any = {
          url: 'test'
        };
        expect(authGuard.canActivate(null, state)).toBeFalsy();
        expect(router.navigate).toHaveBeenCalledWith(['login']);
        expect(authService.redirectUrl).toEqual('test');

        MockControllerService.isLoggedIn = true;
        // Routing should be allowed now
        expect(authGuard.canActivate(null, state)).toBeTruthy();
      })
    );
});
