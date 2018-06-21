import { TestBed, inject, getTestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule, HttpTestingController } from '@angular/common/http/testing';

/**-----
 * Venice imports
 */
import { ControllerService } from './controller.service';
import { NodesService } from './nodes.service';

describe('NodesService', () => {

    let injector: TestBed;
    let service: NodesService;
    let httpMock: HttpTestingController;

    beforeEach(() => {
        TestBed.configureTestingModule({
            providers: [NodesService],
            imports: [
                HttpClientTestingModule,
                RouterTestingModule,
            ]
        });
        injector = getTestBed();
        service = injector.get(NodesService);
        httpMock = injector.get(HttpTestingController);
    });

    it('should be created', inject([NodesService], (svc: NodesService) => {
        expect(svc).toBeTruthy();
    }));
});
