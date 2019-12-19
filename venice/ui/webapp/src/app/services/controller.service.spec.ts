import { HttpClientTestingModule } from '@angular/common/http/testing';
import { inject, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';

import { ControllerService } from './controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { ConfirmationService } from 'primeng/primeng';
import { MessageService, Message, RemoveMessageReq } from './message.service';
import { Utility } from '@app/common/Utility';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { Eventtypes } from '@app/enum/eventtypes.enum';

interface ToasterTestCase {
  description: string;
  testFunction: () => void;
  addToasterCalls?: Message[];
  removeToasterCalls?: RemoveMessageReq[];
  clearToasterCallCount?: number;
}


describe('ControllerService', () => {
  const _ = Utility.getLodash();

  const createError = (msg: string, statusCode: number) => {
    return { body: { message: msg }, statusCode: statusCode };
  };

  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService
      ],
      imports: [
        HttpClientTestingModule,
        RouterTestingModule,
      ]
    });
  });

  it('invoke toaster', inject([ControllerService], (service: ControllerService) => {
    const signoutButton: ToolbarButton = {
      text: 'Sign out',
      callback: () => {
        this.publish(Eventtypes.LOGOUT, { 'reason': 'User logged out' });
      },
      cssClass: 'global-button-primary'
    };

    spyOn(service, 'createSignOutButton').and.returnValue(
      signoutButton
    );
    // invoke success attempting to clear error toasters

    // adds toaster with severity success
    // info toaster
    // error toaster
    // remove toaster

    // with 400 error
    // with 401
    // with 403
    // with 500
    // with 503
    // with no error code

    // webSocketErrorToaster can get user obj
    // webSocketError cant get user obj


    // test function
    // expected call to error toaster
    const dummyButton: ToolbarButton = {
      text: 'dummy',
      callback: () => {}
    };


    const messageService = TestBed.get(MessageService);
    const addSpy = spyOn(messageService, 'add');
    const removeSpy = spyOn(messageService, 'remove');
    const clearSpy = spyOn(messageService, 'clear');
    const testcases: ToasterTestCase[] = [
      {
        description: 'success toaster',
        testFunction: () => {
          service.invokeSuccessToaster('success summary', 'success detail', false, [dummyButton] );
        },
        addToasterCalls: [{
          severity: 'success',
          summary: 'success summary',
          detail: 'success detail',
          buttons: [dummyButton]
        }]
      },
      {
        description: 'success toaster with clear',
        testFunction: () => {
          service.invokeSuccessToaster('success summary', 'success detail', true, [dummyButton] );
        },
        addToasterCalls: [{
          severity: 'success',
          summary: 'success summary',
          detail: 'success detail',
          buttons: [dummyButton]
        }],
        clearToasterCallCount: 1,
      },
      {
        description: 'info toaster',
        testFunction: () => {
          service.invokeInfoToaster('info summary', 'info detail', [dummyButton] );
        },
        addToasterCalls: [{
          severity: 'info',
          summary: 'info summary',
          detail: 'info detail',
          buttons: [dummyButton]
        }],
      },
      {
        description: 'error toaster',
        testFunction: () => {
          service.invokeErrorToaster('error summary', 'error detail', [dummyButton] );
        },
        addToasterCalls: [{
          severity: 'error',
          summary: 'error summary',
          detail: 'error detail',
          sticky: true,
          buttons: [dummyButton]
        }],
      },
      {
        description: 'REST error 400, remove same summary',
        testFunction: () => {
          service.invokeRESTErrorToaster('error summary', createError('error detail', 400));
          service.invokeRESTErrorToaster('error summary', createError('error detail 2', 400));
        },
        addToasterCalls: [
          {
            severity: 'error',
            summary: 'error summary',
            detail: 'error detail',
            sticky: true,
            buttons: []
          },
          {
            severity: 'error',
            summary: 'error summary',
            detail: 'error detail 2',
            sticky: true,
            buttons: []
          },
        ],
        removeToasterCalls: [
          {
            summary: 'error summary',
            detail: null,
          },
          {
            summary: 'error summary',
            detail: null,
          }
        ]
      },
      {
        description: 'REST error 400',
        testFunction: () => {
          service.invokeRESTErrorToaster('error summary', createError('error detail', 400), false);
          service.invokeRESTErrorToaster('error summary', createError('error detail 2', 400), false);
        },
        addToasterCalls: [
          {
            severity: 'error',
            summary: 'error summary',
            detail: 'error detail',
            sticky: true,
            buttons: []
          },
          {
            severity: 'error',
            summary: 'error summary',
            detail: 'error detail 2',
            sticky: true,
            buttons: []
          },
        ],
      },
      // To impl
      {
        description: 'REST error 401',
        testFunction: () => {
          service.invokeRESTErrorToaster('error summary', createError('error detail', 401), false);
        },
        addToasterCalls: [
          {
            severity: 'error',
            summary: Utility.VENICE_CONNECT_FAILURE_SUMMARY,
            detail: 'Your credentials have expired.\nPlease sign in again.',
            sticky: true,
            buttons: [signoutButton]
          },
        ],
      },
      {
        description: 'REST error 403',
        testFunction: () => {
          service.invokeRESTErrorToaster('error summary', createError('error detail', 403), false);
        },
        addToasterCalls: [
          {
            severity: 'error',
            summary: Utility.VENICE_CONNECT_FAILURE_SUMMARY,
            detail: 'Your authorization is insufficient.\nPlease check with your system administrator.',
            sticky: true,
            buttons: []
          },
        ],
      },
      {
        description: 'REST error 5xx',
        testFunction: () => {
          service.invokeRESTErrorToaster('error summary', createError('error detail', 500), false);
          service.invokeRESTErrorToaster('error summary', createError('error detail', 503), false);
        },
        addToasterCalls: [
          {
            severity: 'error',
            summary: Utility.VENICE_CONNECT_FAILURE_SUMMARY,
            detail: 'Venice encountered an internal error.\nSome services may be temporarily unavailable.\nPlease check with your system administrator.',
            sticky: true,
            buttons: []
          },
          {
            severity: 'error',
            summary: Utility.VENICE_CONNECT_FAILURE_SUMMARY,
            detail: 'Venice encountered an internal error.\nSome services may be temporarily unavailable.\nPlease check with your system administrator.',
            sticky: true,
            buttons: []
          },
        ],
      },
      {
        description: 'error status code 0',
        testFunction: () => {
          service.invokeRESTErrorToaster('error summary', createError('error detail', 0), false);
        },
        addToasterCalls: [
          {
            severity: 'error',
            summary: Utility.VENICE_CONNECT_FAILURE_SUMMARY,
            detail: 'Please clear your browser cache and sign in again.',
            sticky: true,
            buttons: [signoutButton]
          },
        ],
      }
    ];


    testcases.forEach((testCase) => {
      testCase.testFunction();

      if (testCase.addToasterCalls) {
        expect(addSpy.calls.count()).toBe(testCase.addToasterCalls.length, testCase.description + ': add toaster call count did not match');

        testCase.addToasterCalls.forEach( (tc, i) => {
          expect(_.isEqual(addSpy.calls.argsFor(i)[0], tc)).toBeTruthy(testCase.description + ': add toaster call did not match index ' + i);
        });

        addSpy.calls.reset();
      }
      if (testCase.removeToasterCalls) {
        expect(removeSpy.calls.count()).toBe(testCase.removeToasterCalls.length, testCase.description + ': remove toaster call count did not match');

        testCase.removeToasterCalls.forEach( (tc, i) => {
          expect(_.isEqual(removeSpy.calls.argsFor(i)[0], tc)).toBeTruthy(testCase.description + ': remove toaster call did not match index ' + i);
        });

        removeSpy.calls.reset();
      }
      if (testCase.clearToasterCallCount) {
        expect(clearSpy.calls.count()).toBe(testCase.clearToasterCallCount, testCase.description + ': clear toaster call count did not match');

        clearSpy.calls.reset();
      }
    });

  }));
});
