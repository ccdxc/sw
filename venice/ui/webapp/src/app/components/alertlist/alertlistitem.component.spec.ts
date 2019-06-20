import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { AlertlistitemComponent } from './alertlistitem.component';
import { SharedModule } from '@app/components/shared//shared.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('AlertlistitemComponent', () => {
  let component: AlertlistitemComponent;
  let fixture: ComponentFixture<AlertlistitemComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [AlertlistitemComponent],
      imports: [
        // Other modules...
        SharedModule,
        MaterialdesignModule
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertlistitemComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
