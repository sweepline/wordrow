import React, { ReactElement } from 'react';
import { Language } from '../language';
import './end-screen.scss';

interface EndScreenProps {
    language: Language;
    qualified: boolean;
    showContinue: boolean;
    score: number;
}

const EndScreen = ({ language, qualified, score, showContinue }: EndScreenProps) => {
    // ------------------------------------------------------------------------
    // TRANSLATIONS

    let success_text, failed_text: ReactElement;
    let continue_text: ReactElement;

    switch (language) {
    case Language.DK:
        success_text  = <>Tillykke! Du er kvalificeret til den næste runde.</>;
        failed_text   = <>Desværre! Find et ord, der bruger alle bogstaver, for at kvalificere til den næste runde.</>;
        continue_text = <>Tryk <b>enter</b> for at fortsætte.</>
        break;
    case Language.GB:
        success_text  = <>Congratulations! You qualify for the next round.</>;
        failed_text   = <>Sorry! Find a word that uses all letters to qualify for the next round.</>;
        continue_text = <>Press <b>enter</b> to continue.</>;
        break;
    default:
        throw new Error(`Unknown Language: ${language}`);
    }

    // ------------------------------------------------------------------------
    // VISUAL

    return (
        <div className="EndScreen">
            { qualified ? success_text : failed_text}

            <div className="Score">{Math.round(score).toLocaleString()}</div>

            <div className={"Continue " + (showContinue ? "show" : "hide")}>
                {continue_text}
            </div>
        </div>
    );
}

export default EndScreen;
