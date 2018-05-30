/* ---------------------------------------------------
    Angular JS libraries
----------------------------------------------------- */
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';

/* ---------------------------------------------------
    Venice App libraries
----------------------------------------------------- */
import { ToolbarComponent } from './toolbar.component';
import { ControllerService } from '@app/services/controller.service';
import { MatIconRegistry } from '@angular/material';
/* ---------------------------------------------------
    Third-party libraries
----------------------------------------------------- */
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';

describe('ToolbarComponent', () => {
  let component: ToolbarComponent;
  let fixture: ComponentFixture<ToolbarComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ToolbarComponent],
      imports: [
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule
      ],
      providers: [
        ControllerService,
        MatIconRegistry
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ToolbarComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
